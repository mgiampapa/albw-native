/* The bottom screen's leased panel (drmlease.h).
 *
 * Written from the specifications: the Wayland protocol drm-lease-v1 (wayland-protocols, staging,
 * MIT; vendored in third_party/wayland), the kernel's KMS interface as its uAPI headers <drm/drm.h>
 * and <drm/drm_mode.h> describe it (MIT), and atomic mode setting as the kernel documentation
 * ("Kernel Mode Setting", "Userland interfaces") explains it.
 *
 * 1. Lease: connect to the compositor, bind wp_drm_lease_device_v1, collect the connectors it offers,
 *    ask for the wanted one and wait for the lease's file descriptor (a DRM master that owns only the
 *    leased connector, CRTC and plane).
 * 2. Light: find the connector's mode and orientation, the leased CRTC and plane, make the buffers
 *    (dumb buffers, exported as dma-bufs for the GPU), and turn the panel on with one atomic commit.
 * 3. Show: a flip thread takes the newest finished picture and commits it as soon as the panel shows
 *    the one before, with the GPU's fence (IN_FENCE_FD) so the kernel waits for the drawing.
 * libwayland-client is opened at run time: the program needs it only where a compositor leases. */
#define _GNU_SOURCE           /* pipe2 */
#define _FILE_OFFSET_BITS 64  /* the 32-bit build: dumb buffer mmap offsets lie above 4 GiB */
#include "drmlease.h"

#ifdef ALBW_DRM_LEASE

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_mode.h>

#include "wayland-client-core.h"

#define LOG(...) fprintf(stderr, "drm lease: " __VA_ARGS__)

/* ---------------------------------------------------------------- libwayland-client, opened at run
 * time. The protocol headers' inline request functions call these names: each is a pointer here. */
#define WL_SYMBOLS(S)                                                                                   \
    S(wl_display_connect) S(wl_display_disconnect) S(wl_display_get_fd) S(wl_display_flush)            \
    S(wl_display_dispatch_pending) S(wl_display_prepare_read) S(wl_display_read_events)                 \
    S(wl_display_cancel_read) S(wl_proxy_marshal_flags) S(wl_proxy_add_listener) S(wl_proxy_destroy)    \
    S(wl_proxy_get_version)
#define S(name) static __typeof__(&name) p_##name;
WL_SYMBOLS(S)
#undef S
static const struct wl_interface *p_wl_registry_interface, *p_wl_callback_interface;
#define wl_display_connect (*p_wl_display_connect)
#define wl_display_disconnect (*p_wl_display_disconnect)
#define wl_display_get_fd (*p_wl_display_get_fd)
#define wl_display_flush (*p_wl_display_flush)
#define wl_display_dispatch_pending (*p_wl_display_dispatch_pending)
#define wl_display_prepare_read (*p_wl_display_prepare_read)
#define wl_display_read_events (*p_wl_display_read_events)
#define wl_display_cancel_read (*p_wl_display_cancel_read)
#define wl_proxy_marshal_flags (*p_wl_proxy_marshal_flags)
#define wl_proxy_add_listener (*p_wl_proxy_add_listener)
#define wl_proxy_destroy (*p_wl_proxy_destroy)
#define wl_proxy_get_version (*p_wl_proxy_get_version)
#define wl_registry_interface (*p_wl_registry_interface)
#define wl_callback_interface (*p_wl_callback_interface)
#include "wayland-client-protocol.h"
#include "drm-lease-v1-client-protocol.h"

static bool load_wayland(void) {
    void *lib = dlopen("libwayland-client.so.0", RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        LOG("libwayland-client.so.0 not found (%s)\n", dlerror());
        return false;
    }
    bool ok = true;
#define S(name) ok = (*(void **)&p_##name = dlsym(lib, #name)) != NULL && ok;
    WL_SYMBOLS(S)
#undef S
    p_wl_registry_interface = dlsym(lib, "wl_registry_interface");
    p_wl_callback_interface = dlsym(lib, "wl_callback_interface");
    ok = ok && p_wl_registry_interface && p_wl_callback_interface;
    if (!ok) LOG("libwayland-client lacks a function this needs\n");
    return ok;
}

/* ---------------------------------------------------------------- state */
typedef struct {
    struct wp_drm_lease_connector_v1 *proxy;
    char name[32];
    uint32_t id;
    bool done, withdrawn;
} Offer;

typedef struct {
    uint32_t handle, fb, pitch;
    uint64_t size;
    int dmabuf;                  /* -1: none */
    struct gbm_bo *bo;           /* tests: a GBM buffer instead of a dumb one */
} Buffer;

#define MAX_OFFERS 8
static struct {
    /* the lease */
    struct wl_display *wl;
    struct wp_drm_lease_device_v1 *device;
    Offer offer[MAX_OFFERS];
    int noffers;
    bool listed, answered, ended;
    int fd;                      /* the lease: a DRM master over the leased objects */
    /* the panel */
    uint32_t connector, crtc, plane;
    struct drm_mode_modeinfo mode;
    int rotation;
    struct {
        uint32_t conn_crtc, crtc_mode, crtc_active, fb, crtc, src[4], dst[4], in_fence;
    } prop;
    Buffer buf[DRMLEASE_BUFFERS];
    /* the mailbox (lock): the buffer on the panel, the one flipping to it, the newest finished */
    pthread_mutex_t lock;
    int shown, flipping, ready, ready_fence;
    bool gone;
    pthread_t thread;
    bool thread_on;
    int stop[2];
    int frames, flips;           /* for the log */
    /* tests */
    bool test;
    int test_fd;
    struct gbm_device *gbm;
    char test_ppm[256];
} D = {.fd = -1, .flipping = -1, .ready = -1, .ready_fence = -1, .lock = PTHREAD_MUTEX_INITIALIZER,
       .stop = {-1, -1}, .test_fd = -1};

static double now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static void gone(const char *why) {
    if (!D.gone) LOG("the panel is lost (%s): the bottom screen is no longer shown\n", why);
    D.gone = true;
}

/* ---------------------------------------------------------------- 1. the lease */
static void offer_name(void *data, struct wp_drm_lease_connector_v1 *c, const char *name) {
    Offer *o = data;
    snprintf(o->name, sizeof o->name, "%s", name);
}
static void offer_description(void *data, struct wp_drm_lease_connector_v1 *c, const char *text) {}
static void offer_id(void *data, struct wp_drm_lease_connector_v1 *c, uint32_t id) { ((Offer *)data)->id = id; }
static void offer_done(void *data, struct wp_drm_lease_connector_v1 *c) { ((Offer *)data)->done = true; }
static void offer_withdrawn(void *data, struct wp_drm_lease_connector_v1 *c) { ((Offer *)data)->withdrawn = true; }
static const struct wp_drm_lease_connector_v1_listener offer_events = {offer_name, offer_description, offer_id,
                                                                        offer_done, offer_withdrawn};

static void device_drm_fd(void *data, struct wp_drm_lease_device_v1 *d, int32_t fd) {
    close(fd);   /* a read-only fd for listing resources: the lease brings its own */
}
static void device_connector(void *data, struct wp_drm_lease_device_v1 *d, struct wp_drm_lease_connector_v1 *c) {
    if (D.noffers == MAX_OFFERS) {
        wp_drm_lease_connector_v1_destroy(c);
        return;
    }
    Offer *o = &D.offer[D.noffers++];
    o->proxy = c;
    wp_drm_lease_connector_v1_add_listener(c, &offer_events, o);
}
static void device_done(void *data, struct wp_drm_lease_device_v1 *d) { D.listed = true; }
static void device_released(void *data, struct wp_drm_lease_device_v1 *d) {}
static const struct wp_drm_lease_device_v1_listener device_events = {device_drm_fd, device_connector, device_done,
                                                                      device_released};

static void registry_global(void *data, struct wl_registry *r, uint32_t name, const char *iface, uint32_t version) {
    if (!D.device && !strcmp(iface, wp_drm_lease_device_v1_interface.name)) {
        D.device = wl_registry_bind(r, name, &wp_drm_lease_device_v1_interface, 1);
        wp_drm_lease_device_v1_add_listener(D.device, &device_events, NULL);
    }
}
static void registry_global_remove(void *data, struct wl_registry *r, uint32_t name) {}
static const struct wl_registry_listener registry_events = {registry_global, registry_global_remove};

static bool synced;
static void sync_done(void *data, struct wl_callback *cb, uint32_t serial) { synced = true; }
static const struct wl_callback_listener sync_events = {sync_done};

static void lease_fd(void *data, struct wp_drm_lease_v1 *l, int32_t fd) {
    D.fd = fd;
    D.answered = true;
}
static void lease_finished(void *data, struct wp_drm_lease_v1 *l) {
    D.answered = D.ended = true;
}
static const struct wp_drm_lease_v1_listener lease_events = {lease_fd, lease_finished};

/* read and handle the compositor's events, waiting at most `ms` (0: only what is there) */
static bool wl_pump(int ms) {
    while (wl_display_prepare_read(D.wl) != 0)
        if (wl_display_dispatch_pending(D.wl) < 0) return false;
    wl_display_flush(D.wl);
    struct pollfd p = {wl_display_get_fd(D.wl), POLLIN, 0};
    if (poll(&p, 1, ms) > 0 && (p.revents & POLLIN)) {
        if (wl_display_read_events(D.wl) < 0) return false;
    } else {
        wl_display_cancel_read(D.wl);
        if (p.revents & (POLLERR | POLLHUP)) return false;
    }
    return wl_display_dispatch_pending(D.wl) >= 0;
}

/* pump until *flag, for at most `ms` (a stalled compositor must not hang the game) */
static bool wl_wait(const bool *flag, int ms) {
    double end = now_ms() + ms;
    while (!*flag) {
        double left = end - now_ms();
        if (left <= 0 || !wl_pump((int)left + 1)) return false;
    }
    return true;
}

static bool internal_panel(const char *name) {
    return !strncmp(name, "DSI-", 4) || !strncmp(name, "eDP-", 4) || !strncmp(name, "LVDS-", 5) ||
           !strncmp(name, "DPI-", 4);
}

static bool get_lease(const char *want) {
    const char *gs = getenv("GAMESCOPE_WAYLAND_DISPLAY");   /* gamescope hides WAYLAND_DISPLAY from games */
    D.wl = gs ? wl_display_connect(gs) : NULL;
    if (!D.wl) D.wl = wl_display_connect(NULL);
    if (!D.wl) {
        LOG("no compositor to ask\n");
        return false;
    }
    struct wl_registry *reg = wl_display_get_registry(D.wl);
    wl_registry_add_listener(reg, &registry_events, NULL);
    struct wl_callback *cb = wl_display_sync(D.wl);
    wl_callback_add_listener(cb, &sync_events, NULL);
    bool ok = wl_wait(&synced, 5000);
    wl_callback_destroy(cb);
    if (!ok || !D.device) {
        LOG("%s\n", ok ? "the compositor leases nothing" : "the compositor does not answer");
        return false;
    }
    if (!wl_wait(&D.listed, 5000)) {
        LOG("the compositor never finished listing its connectors\n");
        return false;
    }
    Offer *pick = NULL;
    for (int i = 0; i < D.noffers; i++) {
        Offer *o = &D.offer[i];
        LOG("offered %s (connector %u)%s\n", o->name, o->id, o->withdrawn ? ", withdrawn" : "");
        if (!pick && o->done && !o->withdrawn && (!strcmp(want, "auto") ? internal_panel(o->name) : !strcmp(o->name, want)))
            pick = o;
    }
    if (!pick) {
        LOG("%s is not offered\n", want);
        return false;
    }
    D.connector = pick->id;
    struct wp_drm_lease_request_v1 *req = wp_drm_lease_device_v1_create_lease_request(D.device);
    wp_drm_lease_request_v1_request_connector(req, pick->proxy);
    struct wp_drm_lease_v1 *lease = wp_drm_lease_request_v1_submit(req);
    wp_drm_lease_v1_add_listener(lease, &lease_events, NULL);
    if (!wl_wait(&D.answered, 5000) || D.fd < 0) {
        LOG("the lease of %s was refused\n", pick->name);
        return false;
    }
    LOG("leased %s (connector %u)\n", pick->name, D.connector);
    return true;
}

/* ---------------------------------------------------------------- 2. the panel */
static int kms(unsigned long request, void *arg) {
    int r;
    do r = ioctl(D.fd, request, arg);
    while (r < 0 && (errno == EINTR || errno == EAGAIN));
    return r;
}

/* the property of object `obj` named `name`: its id (0: none), and its value in *value */
static uint32_t property(uint32_t obj, uint32_t type, const char *name, uint64_t *value) {
    struct drm_mode_obj_get_properties op = {.obj_id = obj, .obj_type = type};
    if (kms(DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &op) < 0 || !op.count_props) return 0;
    uint32_t *ids = calloc(op.count_props, sizeof *ids);
    uint64_t *vals = calloc(op.count_props, sizeof *vals);
    uint32_t found = 0;
    op.props_ptr = (uintptr_t)ids;
    op.prop_values_ptr = (uintptr_t)vals;
    if (ids && vals && kms(DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &op) == 0)
        for (uint32_t i = 0; i < op.count_props && !found; i++) {
            struct drm_mode_get_property gp = {.prop_id = ids[i]};
            if (kms(DRM_IOCTL_MODE_GETPROPERTY, &gp) == 0 && !strcmp(gp.name, name)) {
                found = ids[i];
                if (value) *value = vals[i];
            }
        }
    free(ids);
    free(vals);
    return found;
}

/* "panel orientation" (an enum property): the name of its current value -> degrees */
static int orientation(void) {
    uint64_t v = 0;
    uint32_t id = property(D.connector, DRM_MODE_OBJECT_CONNECTOR, "panel orientation", &v);
    if (!id) return 0;
    struct drm_mode_get_property gp = {.prop_id = id};
    if (kms(DRM_IOCTL_MODE_GETPROPERTY, &gp) < 0 || !gp.count_enum_blobs) return 0;
    struct drm_mode_property_enum *en = calloc(gp.count_enum_blobs, sizeof *en);
    int deg = 0;
    gp.enum_blob_ptr = (uintptr_t)en;
    gp.count_values = 0;
    if (en && kms(DRM_IOCTL_MODE_GETPROPERTY, &gp) == 0)
        for (uint32_t i = 0; i < gp.count_enum_blobs; i++)
            if (en[i].value == v) {
                deg = !strcmp(en[i].name, "Upside Down") ? 180 : !strcmp(en[i].name, "Left Side Up") ? 90
                    : !strcmp(en[i].name, "Right Side Up") ? 270 : 0;
            }
    free(en);
    return deg;
}

/* the connector's preferred mode (else its first) */
static bool panel_mode(void) {
    struct drm_mode_get_connector gc = {.connector_id = D.connector};
    if (kms(DRM_IOCTL_MODE_GETCONNECTOR, &gc) < 0 || !gc.count_modes) return false;
    struct drm_mode_modeinfo *m = calloc(gc.count_modes, sizeof *m);
    if (!m) return false;
    struct drm_mode_get_connector again = {.connector_id = D.connector, .count_modes = gc.count_modes,
                                           .modes_ptr = (uintptr_t)m};
    bool ok = kms(DRM_IOCTL_MODE_GETCONNECTOR, &again) == 0 && again.count_modes;
    if (ok) {
        uint32_t k = 0;
        for (uint32_t i = 0; i < again.count_modes && i < gc.count_modes; i++)
            if (m[i].type & DRM_MODE_TYPE_PREFERRED) { k = i; break; }
        D.mode = m[k];
    }
    free(m);
    return ok;
}

#define PLANE_PRIMARY 1   /* the value of a plane's "type" property for a primary plane */

/* the leased CRTC and a plane that can show on it (a primary plane when the lease has one; the
 * lessee's view numbers the leased CRTCs from 0) */
static bool panel_objects(void) {
    uint32_t crtcs[8];
    struct drm_mode_card_res res = {.crtc_id_ptr = (uintptr_t)crtcs, .count_crtcs = 8};
    if (kms(DRM_IOCTL_MODE_GETRESOURCES, &res) < 0 || !res.count_crtcs) return false;
    D.crtc = crtcs[0];
    uint32_t planes[16];
    struct drm_mode_get_plane_res pr = {.plane_id_ptr = (uintptr_t)planes, .count_planes = 16};
    if (kms(DRM_IOCTL_MODE_GETPLANERESOURCES, &pr) < 0) return false;
    for (uint32_t i = 0; i < pr.count_planes && i < 16; i++) {
        struct drm_mode_get_plane gp = {.plane_id = planes[i]};
        if (kms(DRM_IOCTL_MODE_GETPLANE, &gp) < 0 || !(gp.possible_crtcs & 1)) continue;
        uint64_t type = 0;
        property(planes[i], DRM_MODE_OBJECT_PLANE, "type", &type);
        if (!D.plane || type == PLANE_PRIMARY) D.plane = planes[i];
        if (type == PLANE_PRIMARY) break;
    }
    return D.plane != 0;
}

static bool find_properties(void) {
    static const char *rect[8] = {"SRC_X", "SRC_Y", "SRC_W", "SRC_H", "CRTC_X", "CRTC_Y", "CRTC_W", "CRTC_H"};
    D.prop.conn_crtc = property(D.connector, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID", NULL);
    D.prop.crtc_mode = property(D.crtc, DRM_MODE_OBJECT_CRTC, "MODE_ID", NULL);
    D.prop.crtc_active = property(D.crtc, DRM_MODE_OBJECT_CRTC, "ACTIVE", NULL);
    D.prop.fb = property(D.plane, DRM_MODE_OBJECT_PLANE, "FB_ID", NULL);
    D.prop.crtc = property(D.plane, DRM_MODE_OBJECT_PLANE, "CRTC_ID", NULL);
    D.prop.in_fence = property(D.plane, DRM_MODE_OBJECT_PLANE, "IN_FENCE_FD", NULL);
    bool ok = D.prop.conn_crtc && D.prop.crtc_mode && D.prop.crtc_active && D.prop.fb && D.prop.crtc;
    for (int i = 0; i < 8; i++)
        ok = ((i < 4 ? D.prop.src : D.prop.dst)[i & 3] = property(D.plane, DRM_MODE_OBJECT_PLANE, rect[i], NULL)) && ok;
    return ok;
}

/* an atomic request: (object, property, value) triples, grouped by object as the kernel wants them */
#define REQ_MAX 24
typedef struct { uint32_t obj[REQ_MAX], nprops[REQ_MAX], prop[REQ_MAX]; uint64_t val[REQ_MAX]; int nobj, n; } Req;
static void req_set(Req *r, uint32_t obj, uint32_t prop, uint64_t val) {
    if (r->n == REQ_MAX) return;
    if (!r->nobj || r->obj[r->nobj - 1] != obj) r->obj[r->nobj] = obj, r->nprops[r->nobj++] = 0;
    r->nprops[r->nobj - 1]++;
    r->prop[r->n] = prop, r->val[r->n++] = val;
}
static int req_commit(const Req *r, uint32_t flags) {
    struct drm_mode_atomic a = {.flags = flags, .count_objs = (uint32_t)r->nobj, .objs_ptr = (uintptr_t)r->obj,
                                .count_props_ptr = (uintptr_t)r->nprops, .props_ptr = (uintptr_t)r->prop,
                                .prop_values_ptr = (uintptr_t)r->val};
    return kms(DRM_IOCTL_MODE_ATOMIC, &a);
}

static bool make_buffer(Buffer *b) {
    struct drm_mode_create_dumb cd = {.width = D.mode.hdisplay, .height = D.mode.vdisplay, .bpp = 32};
    if (kms(DRM_IOCTL_MODE_CREATE_DUMB, &cd) < 0) return false;
    b->handle = cd.handle, b->pitch = cd.pitch, b->size = cd.size;
    struct drm_mode_fb_cmd2 fb = {.width = D.mode.hdisplay, .height = D.mode.vdisplay,
                                  .pixel_format = DRM_FORMAT_XRGB8888, .handles = {cd.handle}, .pitches = {cd.pitch}};
    if (kms(DRM_IOCTL_MODE_ADDFB2, &fb) < 0) return false;
    b->fb = fb.fb_id;
    struct drm_mode_map_dumb md = {.handle = cd.handle};
    if (kms(DRM_IOCTL_MODE_MAP_DUMB, &md) == 0) {   /* black until the first picture */
        void *p = mmap(NULL, cd.size, PROT_WRITE, MAP_SHARED, D.fd, (off_t)md.offset);
        if (p != MAP_FAILED) {
            memset(p, 0, cd.size);
            munmap(p, cd.size);
        }
    }
    struct drm_prime_handle ph = {.handle = cd.handle, .flags = DRM_CLOEXEC | DRM_RDWR};
    if (kms(DRM_IOCTL_PRIME_HANDLE_TO_FD, &ph) < 0) return false;
    b->dmabuf = ph.fd;
    return true;
}

static void plane_rect(Req *r) {
    uint64_t w = D.mode.hdisplay, h = D.mode.vdisplay;
    uint64_t v[8] = {0, 0, w << 16, h << 16, 0, 0, w, h};   /* source in 16.16 fixed point */
    for (int i = 0; i < 4; i++) req_set(r, D.plane, D.prop.src[i], v[i]);
    for (int i = 0; i < 4; i++) req_set(r, D.plane, D.prop.dst[i], v[4 + i]);
}

static bool light_panel(void) {
    struct drm_set_client_cap atomic = {.capability = DRM_CLIENT_CAP_ATOMIC, .value = 1};
    if (kms(DRM_IOCTL_SET_CLIENT_CAP, &atomic) < 0) {
        LOG("no atomic mode setting (%s)\n", strerror(errno));
        return false;
    }
    if (!panel_mode() || !panel_objects() || !find_properties()) {
        LOG("the leased connector %u has no usable mode, CRTC or plane\n", D.connector);
        return false;
    }
    for (int i = 0; i < DRMLEASE_BUFFERS; i++) D.buf[i].dmabuf = -1;
    for (int i = 0; i < DRMLEASE_BUFFERS; i++)
        if (!make_buffer(&D.buf[i])) {
            LOG("cannot make the panel's buffers (%s)\n", strerror(errno));
            return false;
        }
    struct drm_mode_create_blob blob = {.data = (uintptr_t)&D.mode, .length = sizeof D.mode};
    if (kms(DRM_IOCTL_MODE_CREATEPROPBLOB, &blob) < 0) return false;
    Req r = {0};
    req_set(&r, D.connector, D.prop.conn_crtc, D.crtc);
    req_set(&r, D.crtc, D.prop.crtc_mode, blob.blob_id);
    req_set(&r, D.crtc, D.prop.crtc_active, 1);
    req_set(&r, D.plane, D.prop.fb, D.buf[0].fb);
    req_set(&r, D.plane, D.prop.crtc, D.crtc);
    plane_rect(&r);
    int rc = req_commit(&r, DRM_MODE_ATOMIC_ALLOW_MODESET);
    int err = errno;
    struct drm_mode_destroy_blob db = {.blob_id = blob.blob_id};
    kms(DRM_IOCTL_MODE_DESTROYPROPBLOB, &db);
    if (rc < 0) {
        LOG("the panel did not take the mode %s (%s)\n", D.mode.name, strerror(err));
        return false;
    }
    D.shown = 0;
    return true;
}

/* ---------------------------------------------------------------- 3. showing pictures */
/* (lock held) the newest finished picture onto the panel */
static void send_ready(void) {
    Req r = {0};
    req_set(&r, D.plane, D.prop.fb, D.buf[D.ready].fb);
    if (D.ready_fence >= 0 && D.prop.in_fence) req_set(&r, D.plane, D.prop.in_fence, (uint64_t)D.ready_fence);
    if (req_commit(&r, DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT) == 0) {
        D.flipping = D.ready;
        D.flips++;
    } else if (errno == EBUSY) {
        return;   /* the panel is still busy: the next flip event sends it */
    } else {
        char why[64];
        snprintf(why, sizeof why, "flip refused: %s", strerror(errno));
        gone(why);
    }
    if (D.ready_fence >= 0) close(D.ready_fence);
    D.ready = D.ready_fence = -1;
}

static void *flip_thread(void *unused) {
    struct pollfd p[2] = {{D.fd, POLLIN, 0}, {D.stop[0], POLLIN, 0}};
    char ev[1024];
    while (poll(p, 2, -1) >= 0 && !(p[1].revents & POLLIN)) {
        if (!(p[0].revents & POLLIN)) continue;
        ssize_t n = read(D.fd, ev, sizeof ev);
        pthread_mutex_lock(&D.lock);
        for (ssize_t at = 0; at + (ssize_t)sizeof(struct drm_event) <= n;) {
            const struct drm_event *e = (const void *)(ev + at);
            if (e->type == DRM_EVENT_FLIP_COMPLETE && D.flipping >= 0) D.shown = D.flipping, D.flipping = -1;
            if (e->length < sizeof *e) break;
            at += e->length;
        }
        if (D.flipping < 0 && D.ready >= 0 && !D.gone) send_ready();
        pthread_mutex_unlock(&D.lock);
    }
    return NULL;
}

/* ---------------------------------------------------------------- tests: GBM buffers on the render
 * node. libgbm is opened at run time; the few declarations follow Mesa's gbm.h. */
struct gbm_device;
struct gbm_bo;
static struct {
    struct gbm_device *(*create_device)(int);
    void (*device_destroy)(struct gbm_device *);
    struct gbm_bo *(*bo_create)(struct gbm_device *, uint32_t, uint32_t, uint32_t, uint32_t);
    void (*bo_destroy)(struct gbm_bo *);
    int (*bo_get_fd)(struct gbm_bo *);
    uint32_t (*bo_get_stride)(struct gbm_bo *);
    void *(*bo_map)(struct gbm_bo *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *, void **);
    void (*bo_unmap)(struct gbm_bo *, void *);
} G;
#define GBM_BO_USE_RENDERING (1u << 2)
#define GBM_BO_USE_LINEAR (1u << 4)
#define GBM_BO_TRANSFER_READ (1u << 0)

static bool load_gbm(void) {
    void *lib = dlopen("libgbm.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) return false;
    void **f[8] = {(void **)&G.create_device, (void **)&G.device_destroy, (void **)&G.bo_create, (void **)&G.bo_destroy,
                   (void **)&G.bo_get_fd, (void **)&G.bo_get_stride, (void **)&G.bo_map, (void **)&G.bo_unmap};
    static const char *names[8] = {"gbm_create_device", "gbm_device_destroy", "gbm_bo_create", "gbm_bo_destroy",
                                   "gbm_bo_get_fd", "gbm_bo_get_stride", "gbm_bo_map", "gbm_bo_unmap"};
    for (int i = 0; i < 8; i++)
        if (!(*f[i] = dlsym(lib, names[i]))) return false;
    return true;
}

static bool open_test(const char *spec, int *w, int *h) {
    int pw, ph, rot, used = 0;
    if (sscanf(spec, "%dx%d,%d%n", &pw, &ph, &rot, &used) != 3 || pw <= 0 || ph <= 0) {
        LOG("ALBW_DRM_LEASE_TEST=%s: wanted WxH,ROTATION[,PPM]\n", spec);
        return false;
    }
    if (spec[used] == ',') snprintf(D.test_ppm, sizeof D.test_ppm, "%s", spec + used + 1);
    D.test = true;
    D.mode.hdisplay = (uint16_t)pw, D.mode.vdisplay = (uint16_t)ph, D.rotation = rot;
    D.test_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (D.test_fd < 0 || !load_gbm() || !(D.gbm = G.create_device(D.test_fd))) {
        LOG("tests: no GBM device on /dev/dri/renderD128\n");
        return false;
    }
    for (int i = 0; i < DRMLEASE_BUFFERS; i++) {
        Buffer *b = &D.buf[i];
        b->bo = G.bo_create(D.gbm, pw, ph, DRM_FORMAT_XRGB8888, GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
        if (!b->bo || (b->dmabuf = G.bo_get_fd(b->bo)) < 0) {
            LOG("tests: cannot make a %dx%d GBM buffer\n", pw, ph);
            return false;
        }
        b->pitch = G.bo_get_stride(b->bo);
    }
    LOG("test panel %dx%d (GBM buffers), rotation %d\n", pw, ph, rot);
    return true;
}

static void write_test_ppm(void) {
    const Buffer *b = &D.buf[D.shown];
    uint32_t stride = 0;
    void *cookie = NULL;
    const uint8_t *px = G.bo_map(b->bo, 0, 0, D.mode.hdisplay, D.mode.vdisplay, GBM_BO_TRANSFER_READ, &stride, &cookie);
    FILE *f = px ? fopen(D.test_ppm, "wb") : NULL;
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", D.mode.hdisplay, D.mode.vdisplay);
        for (int y = 0; y < D.mode.vdisplay; y++)
            for (int x = 0; x < D.mode.hdisplay; x++) {
                const uint8_t *q = px + (size_t)y * stride + (size_t)x * 4;   /* B, G, R, X */
                fputc(q[2], f), fputc(q[1], f), fputc(q[0], f);
            }
    }
    if (px) G.bo_unmap(b->bo, cookie);
    LOG("test panel -> %s: %s\n", D.test_ppm, f && fclose(f) == 0 ? "written" : "failed");
}

/* ---------------------------------------------------------------- drmlease.h */
static int upright_w(void) { return D.rotation % 180 ? D.mode.vdisplay : D.mode.hdisplay; }
static int upright_h(void) { return D.rotation % 180 ? D.mode.hdisplay : D.mode.vdisplay; }

bool drmlease_open(const char *want, int *w, int *h) {
    const char *test = getenv("ALBW_DRM_LEASE_TEST");
    bool ok;
    if (test) ok = open_test(test, w, h);
    else {
        ok = want && *want && load_wayland() && get_lease(want) && light_panel() && pipe2(D.stop, O_CLOEXEC) == 0 &&
             pthread_create(&D.thread, NULL, flip_thread, NULL) == 0;
        if (ok) {
            D.thread_on = true;
            D.rotation = orientation();
            LOG("%ux%u at %u Hz, orientation %d, flips %s\n", D.mode.hdisplay, D.mode.vdisplay, D.mode.vrefresh,
                D.rotation, D.prop.in_fence ? "wait for the GPU in the kernel" : "wait for the GPU here");
        }
    }
    if (!ok) {
        drmlease_close();
        if (!test && want && *want) LOG("no lease: the bottom screen stays in the window\n");
        return false;
    }
    *w = upright_w(), *h = upright_h();
    return true;
}

int drmlease_rotation(void) { return D.rotation; }

void drmlease_buffer(int i, int *dmabuf, int *pitch, int *w, int *h) {
    *dmabuf = D.buf[i].dmabuf, *pitch = (int)D.buf[i].pitch, *w = D.mode.hdisplay, *h = D.mode.vdisplay;
}

int drmlease_free_buffer(void) {
    if (++D.frames == 600) {
        pthread_mutex_lock(&D.lock);
        LOG("%d of %d frames shown on the panel\n", D.flips, D.frames);
        D.frames = D.flips = 0;
        pthread_mutex_unlock(&D.lock);
    }
    if (D.test) return (D.shown + 1) % DRMLEASE_BUFFERS;
    if (D.gone) return -1;
    if (!wl_pump(0) || D.ended) gone(D.ended ? "the compositor ended the lease" : "the compositor connection broke");
    pthread_mutex_lock(&D.lock);
    int i = 0;   /* a buffer neither on the panel, nor flipping to it, nor waiting to */
    while (i == D.shown || i == D.flipping || i == D.ready) i++;
    if (i == DRMLEASE_BUFFERS) {   /* all three busy: replace the waiting picture, it was never shown */
        i = D.ready;
        if (D.ready_fence >= 0) close(D.ready_fence);
        D.ready = D.ready_fence = -1;
    }
    pthread_mutex_unlock(&D.lock);
    return D.gone ? -1 : i;
}

void drmlease_flip(int i, int fence) {
    if (D.test) {
        D.shown = i, D.flips++;
        if (fence >= 0) close(fence);
        return;
    }
    if (fence >= 0 && !D.prop.in_fence) {   /* no fence property: the GPU must be done before the commit */
        struct pollfd p = {fence, POLLIN, 0};
        poll(&p, 1, 1000);
        close(fence);
        fence = -1;
    }
    pthread_mutex_lock(&D.lock);
    if (D.ready_fence >= 0) close(D.ready_fence);
    D.ready = i, D.ready_fence = fence;
    if (D.flipping < 0 && !D.gone) send_ready();
    pthread_mutex_unlock(&D.lock);
}

void drmlease_close(void) {
    if (D.thread_on) {
        if (write(D.stop[1], "x", 1) != 1) LOG("cannot stop the flip thread\n");
        pthread_join(D.thread, NULL);
        D.thread_on = false;
    }
    if (D.test && D.test_ppm[0] && D.flips) write_test_ppm();
    if (D.ready_fence >= 0) close(D.ready_fence);
    D.ready = D.ready_fence = -1;
    for (int i = 0; i < DRMLEASE_BUFFERS; i++) {
        Buffer *b = &D.buf[i];
        if (b->dmabuf > 0) close(b->dmabuf);
        if (b->bo) G.bo_destroy(b->bo);
        if (b->fb) kms(DRM_IOCTL_MODE_RMFB, &b->fb);
        if (b->handle) {
            struct drm_mode_destroy_dumb dd = {.handle = b->handle};
            kms(DRM_IOCTL_MODE_DESTROY_DUMB, &dd);
        }
        *b = (Buffer){.dmabuf = -1};
    }
    for (int i = 0; i < 2; i++)
        if (D.stop[i] >= 0) close(D.stop[i]), D.stop[i] = -1;
    if (D.gbm) G.device_destroy(D.gbm), D.gbm = NULL;
    if (D.test_fd >= 0) close(D.test_fd), D.test_fd = -1;
    if (D.fd >= 0) close(D.fd), D.fd = -1;   /* the panel goes back to the compositor */
    if (D.wl) wl_display_disconnect(D.wl), D.wl = NULL;
    D.test = false;
}

#endif

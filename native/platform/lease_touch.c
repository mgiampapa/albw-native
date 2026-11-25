/* The leased panel's touchscreen (drmlease.h lease_touch_poll), read from its evdev device as the
 * kernel's input interface describes it (<linux/input.h>, Documentation/input/multi-touch-protocol).
 * One finger: the first contact's position and whether any finger is down. In Game Mode no window
 * receives the bottom panel's touches, so the device itself is read, and grabbed so the compositor
 * does not act on them too. ALBW_TOUCH_TRACE=1 logs how old each finger down / up is when read. */
#include "drmlease.h"
#ifdef ALBW_DRM_LEASE

#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

static struct {
    int fd;                 /* -2: not opened yet, -1: unavailable */
    int min[2], max[2];     /* the x / y axis ranges */
    int rotate;
    int raw[2], down;       /* the device's latest position and finger state */
    float nx, ny;           /* the position in the upright picture, 0..1 */
    int trace, edge;
    struct timeval edge_time;
} T = {.fd = -2};

static void axis_range(int axis, int mt_code, int code) {
    struct input_absinfo a = {0};
    if (ioctl(T.fd, EVIOCGABS(mt_code), &a) < 0 || a.maximum <= a.minimum) ioctl(T.fd, EVIOCGABS(code), &a);
    T.min[axis] = a.minimum;
    T.max[axis] = a.maximum > a.minimum ? a.maximum : a.minimum + 1;
}

static void open_device(const char *path, int rotate) {
    /* the system call rather than open(): started from Steam, the preloaded overlay library made
     * open() fail on input devices (seen on the AYN Thor) */
    T.fd = (int)syscall(SYS_openat, AT_FDCWD, path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (T.fd < 0) {
        fprintf(stderr, "touch device %s: cannot open it\n", path);
        return;
    }
    axis_range(0, ABS_MT_POSITION_X, ABS_X);
    axis_range(1, ABS_MT_POSITION_Y, ABS_Y);
    T.rotate = rotate >= 0 ? rotate : (T.max[0] - T.min[0] >= T.max[1] - T.min[1] ? 0 : 90);   /* landscape axes: as is */
    int grabbed = ioctl(T.fd, EVIOCGRAB, 1) == 0;
    T.trace = getenv("ALBW_TOUCH_TRACE") != NULL;
    if (T.trace) {
        int clock = CLOCK_MONOTONIC;   /* event times on the clock read when the event is picked up */
        ioctl(T.fd, EVIOCSCLOCKID, &clock);
    }
    char name[64] = "?";
    ioctl(T.fd, EVIOCGNAME(sizeof name), name);
    fprintf(stderr, "touch device %s (%s): x %d..%d, y %d..%d, turned %d degrees%s\n", path, name, T.min[0], T.max[0],
            T.min[1], T.max[1], T.rotate, grabbed ? "" : ", already grabbed by another program");
}

static float unit(int v, int axis) {
    float f = (float)(v - T.min[axis]) / (float)(T.max[axis] - T.min[axis]);
    return f < 0 ? 0 : f > 1 ? 1 : f;
}

/* a finished report (SYN_REPORT): the position, turned into the upright picture */
static void report(void) {
    float x = unit(T.raw[0], 0), y = unit(T.raw[1], 1);
    switch (T.rotate) {
    case 90: T.nx = y, T.ny = 1 - x; break;
    case 180: T.nx = 1 - x, T.ny = 1 - y; break;
    case 270: T.nx = 1 - y, T.ny = x; break;
    default: T.nx = x, T.ny = y; break;
    }
}

bool lease_touch_poll(const char *path, int rotate, int *down, float *nx, float *ny) {
    if (T.fd == -2) open_device(path, rotate);
    if (T.fd < 0) return false;
    struct input_event ev[64];
    ssize_t n;
    while ((n = read(T.fd, ev, sizeof ev)) > 0)
        for (ssize_t i = 0; i < n / (ssize_t)sizeof ev[0]; i++) {
            const struct input_event *e = &ev[i];
            if (e->type == EV_ABS && (e->code == ABS_MT_POSITION_X || e->code == ABS_X)) T.raw[0] = e->value;
            else if (e->type == EV_ABS && (e->code == ABS_MT_POSITION_Y || e->code == ABS_Y)) T.raw[1] = e->value;
            else if ((e->type == EV_KEY && e->code == BTN_TOUCH) || (e->type == EV_ABS && e->code == ABS_MT_TRACKING_ID)) {
                T.down = e->type == EV_KEY ? e->value != 0 : e->value >= 0;
                T.edge = 1, T.edge_time = e->time;
            } else if (e->type == EV_SYN && e->code == SYN_REPORT) report();
        }
    if (T.trace && T.edge) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double age = (now.tv_sec - T.edge_time.tv_sec) * 1e3 + (now.tv_nsec / 1e3 - T.edge_time.tv_usec) / 1e3;
        fprintf(stderr, "touch device: finger %s, %.1f ms old when read\n", T.down ? "down" : "up", age);
        T.edge = 0;
    }
    *down = T.down;
    *nx = T.nx;
    *ny = T.ny;
    return true;
}
#endif

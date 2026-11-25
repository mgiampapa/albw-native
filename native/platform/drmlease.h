/* The bottom screen on a panel leased from the compositor (DRM lease, Linux), for systems whose
 * compositor drives only one of two panels. Armada's Game Mode on the AYN Thor: gamescope shows the
 * top panel and offers the bottom one (DSI-1) for lease through the Wayland protocol
 * wp_drm_lease_device_v1. The GPU draws the bottom screen straight into the panel's buffers
 * (platform.c); this file asks for the lease, lights the panel and flips its buffers. The touch
 * reader for the panel's own touchscreen is lease_touch.c. */
#pragma once
#if defined(__linux__)
#define ALBW_DRM_LEASE 1

#include <stdbool.h>
#include <stdint.h>

/* the panel's buffers, handed to the panel as a mailbox (drmlease.c) */
#define DRMLEASE_BUFFERS 3

/* Lease the connector named `want` (such as DSI-1, or "auto": the first internal panel offered) from
 * the compositor at GAMESCOPE_WAYLAND_DISPLAY, else WAYLAND_DISPLAY, and light it black. False
 * (logged) when nothing is leased. *w, *h: the panel's size upright (its mode, turned by its
 * orientation). ALBW_DRM_LEASE_TEST=WxH,ROTATION[,PPM] (tests): no compositor and no panel, a WxH
 * panel of GBM buffers on /dev/dri/renderD128, the last picture written to PPM at drmlease_close. */
bool drmlease_open(const char *want, int *w, int *h);
/* the panel's orientation in degrees, counter-clockwise as DRM's "panel orientation" property
 * counts them (Right Side Up: 270) */
int drmlease_rotation(void);
/* buffer i (0 to DRMLEASE_BUFFERS - 1) as a dma-buf for the GPU to draw into: XRGB8888, linear,
 * `pitch` bytes a row, the panel's own (unturned) w x h */
void drmlease_buffer(int i, int *dmabuf, int *pitch, int *w, int *h);
/* Once a frame: the compositor's events, then the buffer to draw this frame into, or -1 when the
 * panel is gone (no picture this frame). Never waits. */
int drmlease_free_buffer(void);
/* show buffer i once `fence` (a sync file from the GPU, or -1; it is closed here) has signalled.
 * Never waits: the newest picture goes to the panel as soon as the one before is on it. */
void drmlease_flip(int i, int fence);
void drmlease_close(void);

/* The panel's touchscreen, read from its input device `path` (grabbed, so the compositor no longer
 * sees it). Once a frame: *down and the finger's place in the upright picture, 0..1, the device's
 * axes turned by `rotate` degrees (-1: from the axes' shape). False when the device cannot be
 * opened. */
bool lease_touch_poll(const char *path, int rotate, int *down, float *nx, float *ny);

#endif

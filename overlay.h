#ifndef OVERLAY_H
#define OVERLAY_H

#include "viewporter-client-protocol.h"
#include <stdbool.h>
#include <stdint.h>
#include <wayland-client-protocol.h>

struct globals {
  struct overlay *overlays;
  struct wl_compositor *wl_compositor;
  struct wl_display *wl_display;
  struct wl_shm *wl_shm;
  struct wp_viewporter *wp_viewporter;
  struct zwlr_layer_shell_v1 *wlr_layer_shell;
  struct zwlr_screencopy_manager_v1 *wlr_screencopy_manager;
};

struct overlay {
  enum wl_output_transform wl_output_transform;
  enum { PENDING, WAITING, READY, FAILED } capture_status;

  struct globals *globals;
  struct buffer *screencopy_buffer;
  struct buffer *buffer;
  struct wl_output *wl_output;
  struct wl_surface *wl_surface;
  struct wp_viewport *wp_viewport;
  struct zwlr_layer_surface_v1 *wlr_layer_surface;
};

void overlay_show(struct overlay *overlay);
void overlay_destroy(struct overlay *overlay);

#endif

#include "overlay.h"
#include "buffer.h"
#include "viewporter-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <stdint.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-util.h>

static void surface_handle_enter(void *data, struct wl_surface *wl_surface,
                                 struct wl_output *output) {
  struct overlay *overlay = data;
  overlay->capture_status = READY;
}

static void surface_handle_leave(void *data, struct wl_surface *wl_surface,
                                 struct wl_output *output) {}

static const struct wl_surface_listener surface_listener = {
    .enter = surface_handle_enter,
    .leave = surface_handle_leave,
};

static void layer_surface_handle_configure(
    void *data, struct zwlr_layer_surface_v1 *wlr_layer_surface,
    uint32_t serial, uint32_t width, uint32_t height) {
  struct overlay *overlay = data;

  zwlr_layer_surface_v1_ack_configure(wlr_layer_surface, serial);

  wp_viewport_set_destination(overlay->wp_viewport, width, height);

  wp_viewport_set_source(overlay->wp_viewport, wl_fixed_from_int(0),
                         wl_fixed_from_int(0),
                         wl_fixed_from_int(overlay->buffer->width),
                         wl_fixed_from_int(overlay->buffer->height));

  wl_surface_attach(overlay->wl_surface, overlay->buffer->wl_buffer, 0, 0);
  wl_surface_damage_buffer(overlay->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(overlay->wl_surface);
}

static void
layer_surface_handle_closed(void *data,
                            struct zwlr_layer_surface_v1 *wlr_layer_surface) {
  struct overlay *overlay = data;
  overlay_destroy(overlay);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_handle_configure,
    .closed = layer_surface_handle_closed,
};

void overlay_show(struct overlay *overlay) {
  overlay->wl_surface =
      wl_compositor_create_surface(overlay->globals->wl_compositor);

  overlay->wp_viewport = wp_viewporter_get_viewport(
      overlay->globals->wp_viewporter, overlay->wl_surface);

  overlay->wlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      overlay->globals->wlr_layer_shell, overlay->wl_surface,
      overlay->wl_output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "still");

  wl_surface_add_listener(overlay->wl_surface, &surface_listener, overlay);

  zwlr_layer_surface_v1_add_listener(overlay->wlr_layer_surface,
                                     &layer_surface_listener, overlay);

  zwlr_layer_surface_v1_set_anchor(overlay->wlr_layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);

  zwlr_layer_surface_v1_set_exclusive_zone(overlay->wlr_layer_surface, -1);
  wl_surface_commit(overlay->wl_surface);
}

void overlay_destroy(struct overlay *overlay) {
  buffer_destroy(overlay->buffer);
  wp_viewport_destroy(overlay->wp_viewport);
  wl_surface_destroy(overlay->wl_surface);
  zwlr_layer_surface_v1_destroy(overlay->wlr_layer_surface);
  wl_output_release(overlay->wl_output);
}

#include "capture.h"
#include "buffer.h"
#include "image.h"
#include "overlay.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include <stdio.h>
#include <string.h>

static void screencopy_handle_buffer(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
  struct overlay *overlay = data;

  overlay->screencopy_buffer =
      buffer_create(overlay->globals->wl_shm, format, width, height, stride);
}

static void screencopy_handle_flags(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t flags) {}

static pixman_format_code_t wl2pixman_format(enum wl_shm_format wl_shm_format) {
  switch (wl_shm_format) {
  case WL_SHM_FORMAT_ARGB8888:
    return PIXMAN_a8r8g8b8;
  case WL_SHM_FORMAT_XRGB8888:
    return PIXMAN_x8r8g8b8;
  case WL_SHM_FORMAT_XBGR8888:
    return PIXMAN_x8b8g8r8;
  case WL_SHM_FORMAT_ABGR8888:
    return PIXMAN_a8b8g8r8;
  case WL_SHM_FORMAT_XRGB2101010:
    return PIXMAN_x2r10g10b10;
  case WL_SHM_FORMAT_XBGR2101010:
    return PIXMAN_x2b10g10r10;
  case WL_SHM_FORMAT_ARGB2101010:
    return PIXMAN_a2r10g10b10;
  case WL_SHM_FORMAT_ABGR2101010:
    return PIXMAN_a2b10g10r10;
  default:
    return 0;
  }
}

static void screencopy_handle_ready(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
  struct overlay *overlay = data;

  pixman_format_code_t pixman_format =
      wl2pixman_format(overlay->screencopy_buffer->format);

  if (pixman_format == 0) {
    fprintf(stderr, "ERROR: unsupported pixel format: 0x%.8x\n",
            overlay->screencopy_buffer->format);
    overlay->capture_status = FAILED;
    return;
  }

  pixman_image_t *screencopy_image = pixman_image_create_bits_no_clear(
      pixman_format, overlay->screencopy_buffer->width,
      overlay->screencopy_buffer->height, overlay->screencopy_buffer->data,
      overlay->screencopy_buffer->stride);

  pixman_image_t *reverted_transform_image = image_revert_wl_output_transform(
      screencopy_image, overlay->wl_output_transform);
  pixman_image_unref(screencopy_image);

  overlay->buffer = buffer_create(
      overlay->globals->wl_shm, overlay->screencopy_buffer->format,
      pixman_image_get_width(reverted_transform_image),
      pixman_image_get_height(reverted_transform_image),
      pixman_image_get_stride(reverted_transform_image));

  buffer_destroy_once_released(overlay->screencopy_buffer);
  overlay->screencopy_buffer = NULL;

  memcpy(overlay->buffer->data, pixman_image_get_data(reverted_transform_image),
         overlay->buffer->size);

  overlay_show(overlay);
  zwlr_screencopy_frame_v1_destroy(zwlr_screencopy_frame_v1);
}

static void screencopy_handle_failed(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1) {
  struct overlay *overlay = data;
  fprintf(stderr, "ERROR: Failed to capture output\n");
  overlay->capture_status = FAILED;
  zwlr_screencopy_frame_v1_destroy(zwlr_screencopy_frame_v1);
}

static void screencopy_handle_damage(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

static void screencopy_handle_linux_dmabuf(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height) {}

static void screencopy_handle_buffer_done(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1) {
  struct overlay *overlay = data;

  zwlr_screencopy_frame_v1_copy(zwlr_screencopy_frame_v1,
                                overlay->screencopy_buffer->wl_buffer);
}

static const struct zwlr_screencopy_frame_v1_listener screencopy_listener = {
    .buffer = screencopy_handle_buffer,
    .flags = screencopy_handle_flags,
    .ready = screencopy_handle_ready,
    .failed = screencopy_handle_failed,
    .damage = screencopy_handle_damage,
    .linux_dmabuf = screencopy_handle_linux_dmabuf,
    .buffer_done = screencopy_handle_buffer_done,
};

void capture(bool overlay_cursor, struct overlay *overlay) {
  struct zwlr_screencopy_frame_v1 *frame =
      zwlr_screencopy_manager_v1_capture_output(
          overlay->globals->wlr_screencopy_manager, overlay_cursor,
          overlay->wl_output);

  zwlr_screencopy_frame_v1_add_listener(frame, &screencopy_listener, overlay);
  overlay->capture_status = WAITING;
}

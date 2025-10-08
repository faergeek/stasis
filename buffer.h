#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <wayland-client-protocol.h>

struct buffer {
  struct wl_buffer *wl_buffer;
  enum wl_shm_format format;
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  size_t size;
  void *data;
};

struct buffer *buffer_create(struct wl_shm *shm, uint32_t format,
                             uint32_t width, uint32_t height, uint32_t stride);

void buffer_destroy(struct buffer *buffer);
void buffer_destroy_once_released(struct buffer *buffer);

#endif

#include "buffer.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

static int create_shm_file(size_t size) {
  char name[] = "/wl_shm-XXXXXX";
  int retries = 100;

  do {
    --retries;
    randname(name + strlen(name) - 6);
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);

      if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
      }

      return fd;
    }
  } while (retries > 0 && errno == EEXIST);

  return -1;
}

struct buffer *buffer_create(struct wl_shm *wl_shm, uint32_t format,
                             uint32_t width, uint32_t height, uint32_t stride) {
  size_t size = stride * height;

  int fd = create_shm_file(size);
  if (fd == -1) {
    return NULL;
  }

  uint8_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close(fd);
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(wl_shm, fd, size);
  struct wl_buffer *wl_buffer =
      wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool);
  close(fd);

  struct buffer *buffer = malloc(sizeof(*buffer));

  *buffer = (struct buffer){
      .wl_buffer = wl_buffer,
      .format = format,
      .width = width,
      .height = height,
      .stride = stride,
      .size = size,
      .data = data,
  };

  return buffer;
}

void buffer_destroy(struct buffer *buffer) {
  if (buffer) {
    wl_buffer_destroy(buffer->wl_buffer);
    munmap(buffer->data, buffer->size);
    free(buffer);
  }
}

static void buffer_handle_release(void *data, struct wl_buffer *wl_buffer) {
  struct buffer *buffer = data;
  buffer_destroy(buffer);
}

static const struct wl_buffer_listener buffer_listener = {
    .release = buffer_handle_release,
};

void buffer_destroy_once_released(struct buffer *buffer) {
  wl_buffer_add_listener(buffer->wl_buffer, &buffer_listener, buffer);
}

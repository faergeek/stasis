#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

struct globals {
  char *command;
  bool done;
  int status;

  struct wl_array monitors;
  struct wl_compositor *wl_compositor;
  struct wl_display *wl_display;
  struct wl_shm *wl_shm;
  struct zwlr_layer_shell_v1 *wlr_layer_shell;
  struct zwlr_screencopy_manager_v1 *wlr_screencopy_manager;
};

struct screenshot_overlay {
  struct globals *globals;
  struct wl_buffer *wl_buffer;
  struct wl_output *wl_output;
  struct wl_surface *wl_surface;
};

static inline void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

static inline int create_shm_file(off_t size) {
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

static inline void buffer_handle_release(void *data,
                                         struct wl_buffer *wl_buffer) {
  wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = buffer_handle_release,
};

static inline struct wl_buffer *create_buffer(struct wl_shm *shm,
                                              uint32_t format, int32_t width,
                                              int32_t height, int32_t stride) {
  size_t size = stride * height;

  int fd = create_shm_file(size);
  if (fd == -1) {
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer *wl_buffer =
      wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool);
  close(fd);

  wl_buffer_add_listener(wl_buffer, &wl_buffer_listener, NULL);

  return wl_buffer;
}

static inline void
screencopy_handle_buffer(void *data, struct zwlr_screencopy_frame_v1 *frame,
                         uint32_t format, uint32_t width, uint32_t height,
                         uint32_t stride) {
  struct screenshot_overlay *overlay = data;

  if (!overlay->wl_buffer) {
    struct wl_buffer *wl_buffer =
        create_buffer(overlay->globals->wl_shm, format, width, height, stride);

    if (!wl_buffer) {
      overlay->globals->status = EXIT_FAILURE;
      overlay->globals->done = true;
      return;
    }

    overlay->wl_buffer = wl_buffer;
  }

  zwlr_screencopy_frame_v1_copy(frame, overlay->wl_buffer);
}

static inline void
screencopy_handle_flags(void *data, struct zwlr_screencopy_frame_v1 *frame,
                        uint32_t flags) {}

void sync_handle_done(void *data, struct wl_callback *wl_callback,
                      uint32_t callback_data) {
  struct screenshot_overlay *overlay = data;

  overlay->globals->status = system(overlay->globals->command);
  overlay->globals->done = true;
}

struct wl_callback_listener sync_listener = {.done = sync_handle_done};

static inline void layer_surface_handle_configure(
    void *data, struct zwlr_layer_surface_v1 *wlr_layer_surface,
    uint32_t serial, uint32_t width, uint32_t height) {
  struct screenshot_overlay *overlay = data;

  zwlr_layer_surface_v1_ack_configure(wlr_layer_surface, serial);
  wl_surface_attach(overlay->wl_surface, overlay->wl_buffer, 0, 0);
  wl_surface_commit(overlay->wl_surface);

  struct wl_callback *sync_callback =
      wl_display_sync(overlay->globals->wl_display);
  wl_callback_add_listener(sync_callback, &sync_listener, overlay);
}

static inline void
layer_surface_handle_closed(void *data,
                            struct zwlr_layer_surface_v1 *wlr_layer_surface) {
  zwlr_layer_surface_v1_destroy(wlr_layer_surface);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_handle_configure,
    .closed = layer_surface_handle_closed,
};

static inline void
screencopy_handle_ready(void *data, struct zwlr_screencopy_frame_v1 *frame,
                        uint32_t tv_sec_hi, uint32_t tv_sec_lo,
                        uint32_t tv_nsec) {
  struct screenshot_overlay *overlay = data;

  overlay->wl_surface =
      wl_compositor_create_surface(overlay->globals->wl_compositor);
  assert(overlay->wl_surface != NULL);

  struct zwlr_layer_surface_v1 *layer_surface =
      zwlr_layer_shell_v1_get_layer_surface(
          overlay->globals->wlr_layer_shell, overlay->wl_surface,
          overlay->wl_output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "still");
  assert(layer_surface != NULL);

  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);

  int layer_surface_anchor =
      ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
      ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;

  zwlr_layer_surface_v1_set_anchor(layer_surface, layer_surface_anchor);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     overlay);
  wl_surface_commit(overlay->wl_surface);
  zwlr_screencopy_frame_v1_destroy(frame);
}

static inline void
screencopy_handle_failed(void *data, struct zwlr_screencopy_frame_v1 *frame) {
  struct screenshot_overlay *overlay = data;
  fprintf(stderr, "ERROR: Failed to capture output\n");
  overlay->globals->status = EXIT_FAILURE;
  overlay->globals->done = true;
  zwlr_screencopy_frame_v1_destroy(frame);
}

const struct zwlr_screencopy_frame_v1_listener screencopy_listener = {
    .buffer = screencopy_handle_buffer,
    .flags = screencopy_handle_flags,
    .ready = screencopy_handle_ready,
    .failed = screencopy_handle_failed,
};

static inline void registry_handle_global(void *data,
                                          struct wl_registry *wl_registry,
                                          uint32_t name, const char *interface,
                                          uint32_t version) {
  struct globals *globals = data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    globals->wl_compositor =
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    globals->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output **wl_output =
        wl_array_add(&globals->monitors, sizeof(*wl_output));

    *wl_output = wl_registry_bind(wl_registry, name, &wl_output_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    globals->wlr_layer_shell =
        wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) ==
             0) {
    globals->wlr_screencopy_manager = wl_registry_bind(
        wl_registry, name, &zwlr_screencopy_manager_v1_interface, 2);
  }
}

static inline void
registry_handle_global_remove(void *data, struct wl_registry *wl_registry,
                              uint32_t name) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

void usage(FILE *restrict stream, const char bin_name[]) {
  fprintf(stream,
          "Usage: %s [options...] -c <command>\n"
          "\n"
          "  -h           Show help message and quit\n"
          "  -c <command> Shell command (same as sh -c) after exit of\n"
          "               which the screen is unfrozen\n"
          "  -p           Include a pointer (cursor) in a frozen screenshot\n",
          bin_name);
}

int main(int argc, char *argv[]) {
  struct globals globals = {0};
  bool overlay_cursor = false;

  const char *bin_name = argv[0];
  if (bin_name == NULL || strlen(bin_name) == 0) {
    bin_name = "still";
  }

  int option;
  while ((option = getopt(argc, argv, "c:hp")) != -1) {
    switch (option) {
    case 'c':
      globals.command = optarg;
      break;
    case 'h':
      usage(stdout, bin_name);
      return EXIT_SUCCESS;
    case 'p':
      overlay_cursor = true;
      break;
    default:
      usage(stderr, bin_name);
      return EXIT_FAILURE;
    }
  }

  if (!globals.command) {
    fprintf(stderr, "ERROR: a command must be provided -c\n");
    usage(stderr, bin_name);
    return EXIT_FAILURE;
  }

  globals.wl_display = wl_display_connect(NULL);
  if (!globals.wl_display) {
    fprintf(stderr, "ERROR: Failed to connect to a Wayland display\n");
    return EXIT_FAILURE;
  }

  wl_array_init(&globals.monitors);

  struct wl_registry *registry = wl_display_get_registry(globals.wl_display);
  wl_registry_add_listener(registry, &wl_registry_listener, &globals);
  wl_display_roundtrip(globals.wl_display);

  struct wl_output **wl_output;
  wl_array_for_each(wl_output, &globals.monitors) {
    struct zwlr_screencopy_frame_v1 *frame =
        zwlr_screencopy_manager_v1_capture_output(
            globals.wlr_screencopy_manager, overlay_cursor, *wl_output);

    struct screenshot_overlay overlay = (struct screenshot_overlay){
        .globals = &globals,
        .wl_output = *wl_output,
    };

    zwlr_screencopy_frame_v1_add_listener(frame, &screencopy_listener,
                                          &overlay);
  }

  while (!globals.done) {
    if (wl_display_dispatch(globals.wl_display) == -1) {
      break;
    }
  }

  wl_display_disconnect(globals.wl_display);

  return globals.status;
}

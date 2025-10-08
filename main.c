#include "array.h"
#include "capture.h"
#include "overlay.h"
#include "viewporter-client-protocol.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>

static void registry_handle_global(void *data, struct wl_registry *wl_registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  struct globals *globals = data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    globals->wl_compositor =
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    globals->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *wl_output =
        wl_registry_bind(wl_registry, name, &wl_output_interface, 4);

    struct overlay overlay = {
        .globals = globals,
        .wl_output = wl_output,
    };

    array_push(globals->overlays, overlay);
  } else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
    globals->wp_viewporter =
        wl_registry_bind(wl_registry, name, &wp_viewporter_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    globals->wlr_layer_shell =
        wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, 3);
  } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) ==
             0) {
    globals->wlr_screencopy_manager = wl_registry_bind(
        wl_registry, name, &zwlr_screencopy_manager_v1_interface, 3);
  }
}

static void registry_handle_global_remove(void *data,
                                          struct wl_registry *wl_registry,
                                          uint32_t name) {}

static const struct wl_registry_listener wl_registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void output_handle_geometry(void *data, struct wl_output *wl_output,
                                   int32_t x, int32_t y, int32_t physical_width,
                                   int32_t physical_height, int32_t subpixel,
                                   const char *make, const char *model,
                                   int32_t transform) {
  struct overlay *overlay = data;
  overlay->wl_output_transform = transform;
}

static void output_handle_mode(void *data, struct wl_output *wl_output,
                               uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh) {}

static void output_handle_done(void *data, struct wl_output *wl_output) {}

static void output_handle_scale(void *data, struct wl_output *wl_output,
                                int32_t factor) {}

static void output_handle_name(void *data, struct wl_output *wl_output,
                               const char *name) {}

static void output_handle_description(void *data, struct wl_output *wl_output,
                                      const char *description) {}

static const struct wl_output_listener output_listener = {
    .geometry = output_handle_geometry,
    .mode = output_handle_mode,
    .done = output_handle_done,
    .scale = output_handle_scale,
    .name = output_handle_name,
    .description = output_handle_description,
};

void usage(FILE *restrict stream, const char bin_name[]) {
  fprintf(stream,
          "Usage: %s [options...] -c <command>\n"
          "\n"
          "  -h           Show help message and quit\n"
          "  -c <command> Shell command, which will be executed via\n"
          "               \"sh -c <command>\" while the screen is frozen\n"
          "  -p           Include a pointer (cursor) on a frozen screenshot\n",
          bin_name);
}

int main(int argc, char *argv[]) {
  struct globals globals = {0};
  char *command = NULL;
  bool overlay_cursor = false;

  const char *bin_name = argv[0];
  if (bin_name == NULL || strlen(bin_name) == 0) {
    bin_name = "still";
  }

  int option;
  while ((option = getopt(argc, argv, "c:hp")) != -1) {
    switch (option) {
    case 'c':
      command = optarg;
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

  if (!command) {
    fprintf(stderr, "ERROR: a command must be provided via -c flag\n");
    usage(stderr, bin_name);
    return EXIT_FAILURE;
  }

  globals.wl_display = wl_display_connect(NULL);
  if (!globals.wl_display) {
    fprintf(stderr, "ERROR: Failed to connect to a Wayland display\n");
    return EXIT_FAILURE;
  }

  struct wl_registry *registry = wl_display_get_registry(globals.wl_display);
  wl_registry_add_listener(registry, &wl_registry_listener, &globals);

  int status = 0;
  while (status == 0) {
    if (wl_display_dispatch(globals.wl_display) == -1) {
      fprintf(stderr, "ERROR: Failed to dispatch events: %s\n",
              strerror(errno));
      status = EXIT_FAILURE;
      break;
    }

    bool all_ready = true;
    for (size_t i = 0; i < array_length(globals.overlays); i++) {
      struct overlay *overlay = &globals.overlays[i];

      switch (overlay->capture_status) {
      case PENDING:
        wl_output_add_listener(overlay->wl_output, &output_listener, overlay);
        capture(overlay_cursor, overlay);
        all_ready = false;
        break;
      case WAITING:
        all_ready = false;
        break;
      case READY:
        break;
      case FAILED:
        status = EXIT_FAILURE;
        all_ready = false;
        break;
      }
    }

    if (all_ready) {
      break;
    }
  }

  status = status || system(command);

  for (size_t i = 0; i < array_length(globals.overlays); i++) {
    overlay_destroy(&globals.overlays[i]);
  }

  array_free(globals.overlays);

  wl_registry_destroy(registry);
  zwlr_screencopy_manager_v1_destroy(globals.wlr_screencopy_manager);
  zwlr_layer_shell_v1_destroy(globals.wlr_layer_shell);
  wp_viewporter_destroy(globals.wp_viewporter);
  wl_compositor_destroy(globals.wl_compositor);
  wl_shm_destroy(globals.wl_shm);
  wl_display_disconnect(globals.wl_display);

  return status;
}

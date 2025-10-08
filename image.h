#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <pixman.h>
#include <wayland-client-protocol.h>

pixman_image_t *
image_revert_wl_output_transform(pixman_image_t *src,
                                 enum wl_output_transform wl_output_transform);

#endif

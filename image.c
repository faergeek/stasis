#include "image.h"
#include <pixman.h>
#include <stdint.h>

#define PI 3.14159265358979323846

pixman_image_t *
image_revert_wl_output_transform(pixman_image_t *src,
                                 enum wl_output_transform wl_output_transform) {
  int32_t src_width = pixman_image_get_width(src);
  int32_t src_height = pixman_image_get_height(src);

  int32_t dst_width;
  int32_t dst_height;
  if (wl_output_transform % 2) {
    dst_width = src_height;
    dst_height = src_width;
  } else {
    dst_width = src_width;
    dst_height = src_height;
  }

  struct pixman_f_transform f_transform;
  pixman_f_transform_init_identity(&f_transform);
  pixman_f_transform_translate(&f_transform, NULL, -(double)src_width / 2,
                               -(double)src_height / 2);

  double rotation_radians = PI * (wl_output_transform % 4) / 2.0;
  pixman_f_transform_rotate(&f_transform, NULL, round(cos(rotation_radians)),
                            round(sin(rotation_radians)));

  if (wl_output_transform & WL_OUTPUT_TRANSFORM_FLIPPED) {
    pixman_f_transform_scale(&f_transform, NULL, -1, 1);
  }

  pixman_f_transform_translate(&f_transform, NULL, (double)dst_width / 2,
                               (double)dst_height / 2);

  struct pixman_f_transform f_transform_inverted;
  pixman_f_transform_invert(&f_transform_inverted, &f_transform);

  struct pixman_transform transform;
  pixman_transform_from_pixman_f_transform(&transform, &f_transform_inverted);

  pixman_image_set_transform(src, &transform);

  pixman_image_t *dst = pixman_image_create_bits(
      pixman_image_get_format(src), dst_width, dst_height, NULL, 0);

  pixman_image_composite32(PIXMAN_OP_SRC, src, NULL, dst, 0, 0, 0, 0, 0, 0,
                           dst_width, dst_height);

  return dst;
}

#include "unity.h"

#include "image.h"

void setUp(void) {}
void tearDown(void) {}

static void image_revert_wl_output_transform_NORMAL(void) {
  uint32_t src_bits[3][4] = {
      {0xff0000, 0x00ff00, 0x0000ff, 0xff00ff},
      {0x00ff00, 0x0000ff, 0xff00ff, 0xff0000},
      {0x0000ff, 0xff00ff, 0xff0000, 0x00ff00},
  };

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 4, 3, (uint32_t *)src_bits, 4 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_NORMAL);
  pixman_image_unref(src);

  uint32_t expected[3][4] = {
      {0xff0000, 0x00ff00, 0x0000ff, 0xff00ff},
      {0x00ff00, 0x0000ff, 0xff00ff, 0xff0000},
      {0x0000ff, 0xff00ff, 0xff0000, 0x00ff00},
  };

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 4 * 3);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_90(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_90);
  pixman_image_unref(src);

  uint32_t expected[3][4] = {
      {0x00ff00, 0xff0000, 0xff00ff, 0x0000ff},
      {0xff0000, 0xff00ff, 0x0000ff, 0x00ff00},
      {0xff00ff, 0x0000ff, 0x00ff00, 0xff0000},
  };

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 4 * 3);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_180(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_180);
  pixman_image_unref(src);

  uint32_t expected[4][3] = {{0xff00ff, 0xff0000, 0x00ff00},
                             {0x0000ff, 0xff00ff, 0xff0000},
                             {0x00ff00, 0x0000ff, 0xff00ff},
                             {0xff0000, 0x00ff00, 0x0000ff}};

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 3 * 4);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_270(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_270);
  pixman_image_unref(src);

  uint32_t expected[3][4] = {
      {0xff0000, 0x00ff00, 0x0000ff, 0xff00ff},
      {0x00ff00, 0x0000ff, 0xff00ff, 0xff0000},
      {0x0000ff, 0xff00ff, 0xff0000, 0x00ff00},
  };

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 4 * 3);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_flipped(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_FLIPPED);
  pixman_image_unref(src);

  uint32_t expected[4][3] = {{0xff0000, 0x00ff00, 0x0000ff},
                             {0x00ff00, 0x0000ff, 0xff00ff},
                             {0x0000ff, 0xff00ff, 0xff0000},
                             {0xff00ff, 0xff0000, 0x00ff00}};

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 3 * 4);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_flipped_90(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_FLIPPED_90);
  pixman_image_unref(src);

  uint32_t expected[3][4] = {
      {0x0000ff, 0xff00ff, 0xff0000, 0x00ff00},
      {0x00ff00, 0x0000ff, 0xff00ff, 0xff0000},
      {0xff0000, 0x00ff00, 0x0000ff, 0xff00ff},
  };

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 4 * 3);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_flipped_180(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_FLIPPED_180);
  pixman_image_unref(src);

  uint32_t expected[4][3] = {{0x00ff00, 0xff0000, 0xff00ff},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0x0000ff, 0x00ff00, 0xff0000}};

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 3 * 4);
  pixman_image_unref(dst);
}

static void image_revert_wl_output_transform_flipped_270(void) {
  uint32_t src_bits[4][3] = {{0x0000ff, 0x00ff00, 0xff0000},
                             {0xff00ff, 0x0000ff, 0x00ff00},
                             {0xff0000, 0xff00ff, 0x0000ff},
                             {0x00ff00, 0xff0000, 0xff00ff}};

  pixman_image_t *src = pixman_image_create_bits_no_clear(
      PIXMAN_a8r8g8b8, 3, 4, (uint32_t *)src_bits, 3 * 4);

  pixman_image_t *dst =
      image_revert_wl_output_transform(src, WL_OUTPUT_TRANSFORM_FLIPPED_270);
  pixman_image_unref(src);

  uint32_t expected[3][4] = {
      {0xff00ff, 0x0000ff, 0x00ff00, 0xff0000},
      {0xff0000, 0xff00ff, 0x0000ff, 0x00ff00},
      {0x00ff00, 0xff0000, 0xff00ff, 0x0000ff},
  };

  TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, pixman_image_get_data(dst), 4 * 3);
  pixman_image_unref(dst);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(image_revert_wl_output_transform_NORMAL);
  RUN_TEST(image_revert_wl_output_transform_90);
  RUN_TEST(image_revert_wl_output_transform_180);
  RUN_TEST(image_revert_wl_output_transform_270);
  RUN_TEST(image_revert_wl_output_transform_flipped);
  RUN_TEST(image_revert_wl_output_transform_flipped_90);
  RUN_TEST(image_revert_wl_output_transform_flipped_180);
  RUN_TEST(image_revert_wl_output_transform_flipped_270);

  return UNITY_END();
}

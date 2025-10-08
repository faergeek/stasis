#include "unity.h"

#include "array.h"

void setUp(void) {}
void tearDown(void) {}

static void array_length_is_0_if_array_is_NULL(void) {
  uint8_t *array = NULL;
  TEST_ASSERT_EQUAL_size_t(0, array_length(array));
}

static void array_length_is_1_after_pushing_an_element(void) {
  char *array = NULL;
  array_push(array, 'h');
  TEST_ASSERT_EQUAL_size_t(1, array_length(array));
  array_free(array);
}

static void array_push_can_push_a_single_element(void) {
  const int expected[] = {42};
  int8_t *actual = NULL;

  array_push(actual, 42);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actual, array_length(actual));
  array_free(actual);
}

static void array_push_can_push_multiple_elements(void) {
  const int expected[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  };

  size_t length = sizeof(expected) / sizeof(expected[0]);

  int *actual = NULL;

  for (size_t i = 0; i < length; i++) {
    array_push(actual, i);
  }

  TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, length);
  array_free(actual);
}

static void array_free_does_nothing_if_array_is_NULL(void) {
  char *array = NULL;
  array_free(array);
  TEST_ASSERT_NULL(array);
}

static void array_free_frees_memory_if_array_is_not_NULL(void) {
  char *array = NULL;
  array_push(array, 42);
  array_free(array);
  TEST_ASSERT_NULL(array);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(array_length_is_0_if_array_is_NULL);
  RUN_TEST(array_length_is_1_after_pushing_an_element);

  RUN_TEST(array_push_can_push_a_single_element);
  RUN_TEST(array_push_can_push_multiple_elements);

  RUN_TEST(array_free_does_nothing_if_array_is_NULL);
  RUN_TEST(array_free_frees_memory_if_array_is_not_NULL);

  return UNITY_END();
}

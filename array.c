#include "array.h"
#include <string.h>

#define array_header(array)                               \
  ((array) ? ((struct array_header *)(array) - 1) : NULL)

static size_t array_capacity(void *array) {
  return ((array) ? array_header(array)->capacity : 0);
}

size_t array_length(void *array) {
  return ((array) ? array_header(array)->length : 0);
}

void *array_grow_if_needed(void *array, size_t element_size,
                           size_t min_capacity) {
  size_t capacity = array_capacity(array);
  if (capacity >= min_capacity) {
    return array;
  }

  if (min_capacity < 4) {
    min_capacity = 4;
  }

  if (min_capacity < capacity * 2) {
    min_capacity = capacity * 2;
  }

  size_t length = array_length(array);

  struct array_header *new_array =
      malloc(sizeof(*new_array) + min_capacity * element_size);

  *new_array = (struct array_header){
      .capacity = min_capacity,
      .length = length,
  };

  if (array) {
    memcpy(new_array->contents, array, length * element_size);
    array_free(array);
  }

  return new_array->contents;
}

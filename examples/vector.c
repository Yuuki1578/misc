#define MISC_USE_GLOBAL_ALLOCATOR
#define VECTOR_ALLOC_FREQ (256)
#include "../misc.h"
#include <stddef.h>
#include <stdio.h>

int main(void) {
  ARENA_INIT();

  Vector vec = vector_create_with(1, sizeof(size_t));

  for (size_t i = 1 << 14; i > 0; i--)
    vector_push(&vec, size_t, i * 100);

  for (size_t i = 0, *current; i < vec.length; i++) {
    current = vector_get(&vec, i);
    printf("%zu\n", *current);
  }

  printf("Capacity:  %zu\n", vec.capacity);
  printf("Length:    %zu\n", vec.length);
  printf("Remaining: %zu\n", vector_remaining(&vec));

  ARENA_DEINIT();
}

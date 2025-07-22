#include "libmisc/arena.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  Arena arena = {0};
  assert(arena_create(&arena, 10, false));

  char *a = arena_alloc(&arena, 2);
  assert(a);
  strcpy(a, "a");

  printf("String = %s\n", a);
  printf("Size = %zu\n", arena.size);
  printf("Offset = %zu\n", arena.offset);
  printf("Remains = %zu\n\n", arena.size - arena.offset);

  a = arena_realloc(&arena, a, 1, 2);
  assert(a);

  printf("String = %s\n", a);
  printf("Size = %zu\n", arena.size);
  printf("Offset = %zu\n", arena.offset);
  printf("Remains = %zu\n\n", arena.size - arena.offset);

  a = arena_realloc(&arena, a, 2, 6);
  assert(a);

  printf("String = %s\n", a);
  printf("Size = %zu\n", arena.size);
  printf("Offset = %zu\n", arena.offset);
  printf("Remains = %zu\n\n", arena.size - arena.offset);

  a = arena_realloc(&arena, a, 6, 2);
  assert(a);

  printf("String = %s\n", a);
  printf("Size = %zu\n", arena.size);
  printf("Offset = %zu\n", arena.offset);
  printf("Remains = %zu\n\n", arena.size - arena.offset);

  arena_free(&arena);
}

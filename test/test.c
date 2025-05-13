#include <libmisc/arena.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  Arena* arena = &(Arena){};
  if (arena_new(arena, PAGE_SIZE, true) != ARENA_READY) {
    return 1;
  }

  char* string = arena_alloc(arena, 64);
  strcpy(string, "Hello");

  char* another = arena_alloc(arena, 64);
  strcpy(another, __FILE__);
  printf("%s\n", string);
  printf("%s\n", another);

  arena_snapshot(arena);
  arena_dealloc(arena);

  return 0;
}

#include <libmisc/arena.h>
#include <stdio.h>

Arena *arena = &(Arena){0};

int main(void) {
  void *buf;

  ArenaInit(arena, PAGE_SIZE, true);

  buf = ArenaAlloc(arena, PAGE_SIZE);
  buf = ArenaAlloc(arena, PAGE_SIZE);
  buf = ArenaAlloc(arena, PAGE_SIZE);
  buf = ArenaAlloc(arena, PAGE_SIZE);
  buf = ArenaAlloc(arena, PAGE_SIZE);
  buf = ArenaAlloc(arena, PAGE_SIZE);
  (void)buf;

  printf("%zu\n", arena->capacity);

  ArenaDealloc(arena);
}

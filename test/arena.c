#include <libmisc/arena.h>
#include <stdio.h>

int main(void) {
  Arena *arena = ArenaNew(32, true);
  if (arena == NULL)
    return 1;

  (void)ArenaAlloc(arena, 32);
  (void)ArenaRealloc(arena, NULL, 10, 1762);

  printf("%zu\n", ArenaCapacity(arena));
  printf("%zu\n", ArenaOffset(arena));
  printf("%zu\n", ArenaRemaining(arena));

  ArenaDealloc(arena);
}

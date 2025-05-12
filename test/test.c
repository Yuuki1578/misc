#include <libmisc/arena.h>
#include <stdio.h>

int main(void) {
  if (arena_new_global() != ARENA_READY) {
    return 1;
  }

  char* string = arena_alloc_global(PAGE_SIZE);
  strcpy(string, __FILE__);
  printf("%s\n", string);
  arena_snapshot_global();

  arena_dealloc_global();
  return 0;
}

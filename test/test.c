#include <libmisc/arena.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  arena_global_initializer();

  char* s = arena_alloc(64);
  arena_snapshot_global();

  s = arena_alloc(1024);
  arena_snapshot_global();

  s = arena_alloc(12);
  strcpy(s, "Booo!");
  arena_snapshot_global();

  s = arena_realloc(s, 12, 72);
  puts(s);
  arena_snapshot_global();

  arena_dealloc();
}

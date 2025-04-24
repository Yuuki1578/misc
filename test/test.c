// Program example

#include <bits/page_size.h>
#include <libmisc/arena.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  // change default size at runtime
  ARENA_STEP_DFL = PAGE_SIZE * 2;

  // initialize global arena with @ARENA_STEP_DFL
  arena_global_initializer();

  // allocate 64 bytes
  char* s = arena_alloc(64);
  arena_snapshot_global();

  // allocate 1024 bytes
  s = arena_alloc(1024);
  arena_snapshot_global();

  // allocate 12 bytes
  s = arena_alloc(12);
  strcpy(s, "Booo!");
  arena_snapshot_global();

  // reallocate 72 bytes from 12 bytes
  s = arena_realloc(s, 12, 72);

  // content still present
  puts(s);
  arena_snapshot_global();

  // deallocate global arena
  arena_dealloc();
}

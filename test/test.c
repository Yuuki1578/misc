#include <libmisc/Arena.h>
#include <stdio.h>

int main(void) {
  ArenaGlobalInitialize();
  int* p = ArenaGenericAlloc(&ArenaAllocator, sizeof(int));
  *p = 1 << 8;

  int* q = ArenaGenericAlloc(&ArenaAllocator, sizeof(int));
  *q = 1 << 16;

  printf("p: %d\n", *p);
  printf("q: %d\n", *q);
  printf("capacity: %zu\n", ArenaAllocator.capacity);
  printf("position: %zu\n", ArenaAllocator.position);
}

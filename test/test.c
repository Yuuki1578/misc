#include <libmisc/layout.h>
#include <stdio.h>

int main(void) {
  Layout layout = LayoutNew(sizeof(int), 5);
  void *memory = LayoutAlloc(&layout);

  printf("capacity: %zu\n", layout.capacity);
  printf("requested: %zu\n", layout.requested);

  LayoutDealloc(&layout, memory);
}

#include <libmisc/memory.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  auto layout = layout_new(sizeof(char), 32);
  auto ptr = (char *)layout_alloc(&layout);
  strcpy(ptr, "Hello, world!");

  for (int i = 0; i < layout.requested; i++) {
    printf("%d\n", ptr[i]);
  }

  layout_dealloc(&layout, ptr);
}

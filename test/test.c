#include <libmisc/Arena.h>
#include <stdio.h>

int main(void) {
  ArenaGlobalInitialize();
  char* str = ArenaAlloc(14);

  strcpy(str, "Hello, world!");
  printf("address: %p\n", str);

  str = ArenaRealloc(str, 64);
  strcat(str, " Hell yeah! Arena!");
  printf("address: %p\n", str);

  printf("str: %s\n", str);
  printf("capacity: %zu\n", ArenaGetGlobalCapacity());
  printf("position: %zu\n", ArenaGetGlobalPosition());

  ArenaDealloc();
}

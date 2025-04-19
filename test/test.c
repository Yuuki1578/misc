#include <libmisc/Arena.h>
#include <stdio.h>

int main(void) {
  ArenaGlobalInitialize();
  char* string = ArenaAlloc(64);
  char* literal = strncpy(string, "Hello, world!", 14);

  int* large_array = ArenaAlloc(512);

  puts(string);
  puts(literal);

  printf("Capacity: %zu bytes\n", ArenaGetGlobalCapacity());
  printf("Remaining: %zu bytes\n", ArenaGetGlobalRemaining());
  printf("Position: %zu from left\n", ArenaGetGlobalPosition());

  ArenaDealloc();
}

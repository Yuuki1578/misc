#include <libmisc/Arena.h>
#include <stdio.h>

int main(void) {
  ArenaGlobalInitialize();

  char* str = ArenaAlloc(1025);

  if (!str) {
    puts("null");
    return 1;
  }

  strcpy(str, "Hello, world! ");
  puts(str);
  ArenaShowGlobalInformation();

  ArenaDealloc();
}

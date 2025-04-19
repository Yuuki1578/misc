#include <libmisc/Arena.h>
#include <signal.h>
#include <stdio.h>

int main(void) {
  ArenaGlobalInitialize();

  int* p = ArenaAlloc(sizeof(int));
  *p = 1024;

  p = ArenaRealloc(p, sizeof(int) * 1024);

  if (p == nullptr) {
    return SIGSEGV;
  }

  p[1] = *p * 2;

  printf("%d\n", p[0]);
  printf("%d\n", p[1]);

  ArenaDealloc();
}

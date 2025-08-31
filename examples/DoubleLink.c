#define MISC_USE_GLOBAL_ALLOCATOR
#include "../Misc.h"

int main(void) {
  ARENA_INIT();

  DoubleLink Dlink = {
      .ItemSize = 64,
  };

  for (int I = 1; I <= 1 << 12; I++) {
    char Buffer[64] = {};
    sprintf(Buffer, "Iteration: %d", I);
    doubleLinkAppend(&Dlink, Buffer);
  }

  dlinkForward(Dlink, Head) {
    char *Buffer = Head->Item;
    printf("%s\n", Buffer);
  }

  ARENA_DROP();
}

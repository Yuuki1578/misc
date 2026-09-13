#define MISC_IMPL
#include "../misc.h"

int main(void) {
  Arena *arena = arenaNew(1 << 12);

  for (f80 i = 0; i < 1.0; i += 1e-4) {
    tmpPrintf(arena, "%.10f", i);
  }

  arenaFree(arena);
}

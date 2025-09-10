#include "../Misc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define LENGTH_OF(Array) (sizeof(Array) / sizeof *(Array))

static int NumericTable[] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
};

char *createStringFromOrder(Arena *Arena) {
  char *BaseString = arenaAlloc(Arena, 128);
  size_t Offset = 0;

  for (size_t I = 0; I < LENGTH_OF(NumericTable); I++)
    for (int J = 0; J < NumericTable[I]; J++)
      BaseString[Offset++] = NumericTable[I] + '0';

  return BaseString;
}

int main(void) {
  Arena *Context = arenaCreate(ARENA_PAGE);
  assert(Context);

  printf("%s\n", createStringFromOrder(Context));
  arenaFree(Context);
}

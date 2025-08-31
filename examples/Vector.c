#define MISC_USE_GLOBAL_ALLOCATOR
#define VECTOR_ALLOC_FREQ (256)
#include "../Misc.h"
#include <stddef.h>
#include <stdio.h>

int main(void) {
  ARENA_INIT();

  Vector NewVector = vectorCreateWith(1, sizeof(size_t));

  for (size_t I = 1 << 14; I > 0; I--)
    vectorPush(&NewVector, &I);

  for (size_t I = 0, *Current; I < NewVector.Length; I++) {
    Current = vectorGet(&NewVector, I);
    printf("%zu\n", *Current);
  }

  printf("Capacity:  %zu\n", NewVector.Capacity);
  printf("Length:    %zu\n", NewVector.Length);
  printf("Remaining: %zu\n", vectorRemaining(&NewVector));

  ARENA_DROP();
}

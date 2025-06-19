#define VECTOR_UPCOMING
#include <libmisc/vector.h>
#include <stdio.h>

int main(void) {
  Vector vec = VectorNew(sizeof(int));

  if (!VectorReserve(&vec, 32))
    return 1;

  if (!VectorAlign(&vec, sizeof(int)))
    return 2;

  for (int i = 1; i <= 32; i++) {
    VectorPush(&vec, Item(int, i * 100));
  }

  for (size_t i = 0; i < vec.len; i++) {
    int *item = VectorAt(&vec, i);
    if (item == NULL)
      break;

    printf("%d\n", *item);
  }

  VectorFree(&vec);
  return 0;
}

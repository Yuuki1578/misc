#define VECTOR_UPCOMING
#include <libmisc/vector.h>
#include <stdio.h>

int main(void) {
  Vector vec = VectorNew(sizeof(int));

  if (!VectorReserve(&vec, 16))
    return 1;

  if (!VectorAlign(&vec, sizeof(int)))
    return 2;

  for (int i = 1; i <= 512; i++) {
    VectorPush(&vec, pointerof(int, i * 100));
  }

  for (size_t i = 0; i < vec.len; i++) {
    int *item = VectorAt(&vec, i);
    if (item == NULL)
      break;

    printf("%d\n", *item);
  }

  VectorPush(&vec, &(int){69});
  printf("Vector capacity:  %zu\n", vec.cap);
  printf("Vector len:       %zu\n", vec.len);
  printf("Vector item size: %zu\n", vec.item_size);

  VectorFree(&vec);
  return 0;
}

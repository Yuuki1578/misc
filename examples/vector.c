#include "./libmisc/vector.h"
#include <stdio.h>

int main(void) {
  Vector vector = vector_with(1, sizeof(int));

  for (size_t i = 0; i < 1 << 12; i++) {
    int item = (i + 1) * 10;
    vector_push(&vector, &item);
    printf("Capacity: %zu\n", vector.capacity);
  }

  vector_resize(&vector, 32);
  vector_push(&vector, &(int){0});

  // for (size_t i = 0; i < vector.length; i++) {
  //   int *item = vector_at(&vector, i);
  //   printf("Item at: %zu is %d\n", i, *item);
  // }


  // for (size_t i = 0; i < vector.length; i++) {
  //   int *item = vector_at(&vector, i);
  //   printf("Item at: %zu is %d\n", i, *item);
  // }

  // vector_resize(&vector, vector.capacity * 10);

  // for (size_t i = 0; i < vector.length; i++) {
  //   int *item = vector_at(&vector, i);
  //   printf("Item at: %zu is %d\n", i, *item);
  // }

  printf("Capacity: %zu\n", vector.capacity);
  printf("Length: %zu\n", vector.length);
  printf("Remaining: %zu\n", vector_remaining(&vector));

  vector_free(&vector);
  return 0;
}

#define MISC_IMPL
#include "../misc.h"

typedef struct {
  usize *items;
  usize cap, len;
} ArrayOfUsize;

int main(void) {
  ArrayOfUsize array = {0};

  for (usize i = 0; i < 1 << 10; i++)
    // Put in reverse from 0
    arrayAppendAt(&array, 0, i);

  for (usize i = 0; i < array.len; i++)
    printf("index %zu: %zu\n", i, array.items[i]);

  printf("Capacity: %zu\n", array.cap);

  arrayFree(&array);
}

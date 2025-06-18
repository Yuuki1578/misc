#if __STDC_VERSION__ >= 201709L
#define VECTOR_STEP (1024)
#include <libmisc/vector.h>
#include <stdint.h>
#endif

#include <stdio.h>

int main(void) {
#if __STDC_VERSION__ >= 201709L
  vector(uint64_t) big_ints = VECTOR_NEW;

  for (int i = 1; i <= UINT16_MAX; i++) {
    vector_push(big_ints, i * 10);
    printf("%lu\n", (uint64_t)big_ints.elems[i - 1]);
  }

  vector_free(big_ints);
#else
  printf("STDC < 201710L\n");
#endif
  return 0;
}

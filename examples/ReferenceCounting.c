#include <libmisc/ReferenceCounting.h>
#include <stdio.h>

int ThreadRoutine(void *args) {
  int *data = args;
  RefCountStrong(data);
  *data += 1;
  RefCountWeak(data);

  return 0;
}

int main(void) {
  int *data, inc;
  data = RefCountAlloc(sizeof *data);

  for (inc = 0; inc < 10; inc++) {
    thrd_t handle;
    thrd_create(&handle, ThreadRoutine, data);
    printf("%d\n", *data);
  }

  printf("Reference counted: %zu\n", RefCountLifetime(data));
  printf("Now: %d\n", *data);
  RefCountWeak(data);
}

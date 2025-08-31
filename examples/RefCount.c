#include "../Misc.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

int threadRoutine(void *Args) {
  int *Data = Args;
  struct timespec Timer = {.tv_sec = 1};

  // lock
  refcountUpgrade(&Args);
  *Data += 1;

  // lock
  refcountDegrade(&Args);
  printf("thread %d is done!\n"
         "lifetime: %zu\n",

         // lock
         *Data, refcountLifetime(&Args));

  thrd_sleep(&Timer, NULL);
  return 0;
}

int main(void) {
  Vector HandlerList = vectorCreateWith(10, sizeof(thrd_t));
  int *Data = refcountAlloc(sizeof(int));
  void *Holder = Data;

  for (int I = 0; I < 10; I++) {
    thrd_t Handle;
    thrd_create(&Handle, threadRoutine, Data);
    vectorPush(&HandlerList, &Handle);
  }

  for (size_t I = 0; I < HandlerList.Length; I++) {
    thrd_t *Handle = vectorGet(&HandlerList, I);
    thrd_join(*Handle, NULL);
  }

  printf("now: %d\n", *Data);
  vectorFree(&HandlerList);
  refcountDegrade(&Holder);

  printf("lifetime: %zu\n", refcountLifetime(&Holder));
  printf("address: %p\n", Holder);
}

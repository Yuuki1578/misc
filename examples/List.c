#define MISC_USE_GLOBAL_ALLOCATOR
#include "../Misc.h"
#include <stdio.h>

typedef List(long) LongList;

void printList(LongList *source) {
  printf("%zu\n", source->Capacity);
  printf("%zu\n", source->Length);
  listMakeFit(*source);

  printf("%zu\n", source->Capacity);
  printf("%zu\n", source->Length);
}

int main(void) {
  ARENA_INIT();

  LongList SomeList = {0};

  for (long I = 0, J = 1; I < 10000; I++, J++) {
    listAppend(SomeList, J * 10);
    printf("%li\n", SomeList.Items[I]);
  }

  printList((void *)&SomeList);

  ARENA_DROP();
}

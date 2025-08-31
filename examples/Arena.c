#include "../Misc.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  Arena *BaseArena = arenaCreate(ARENA_PAGE);
  char *Cstring = arenaAlloc(BaseArena, 32);
  strcpy(Cstring, "Hello, world!");
  printf("%s\n", Cstring);

  Cstring = arenaRealloc(BaseArena, Cstring, 32, 10);
  Cstring[9] = '\0';
  printf("%s\n", Cstring);

  char *LargeBytes = arenaAlloc(BaseArena, ARENA_PAGE * 2);
  if (LargeBytes != NULL) {
    memset(LargeBytes, 'E', ARENA_PAGE * 2 - 1);
    printf("%s\n", LargeBytes);
    printf("Size: %zu\n", BaseArena->Next->Total);
    printf("Offset: %zu\n", BaseArena->Next->Offset);
    printf("Remains: %zu\n", REMAIN_OF(BaseArena->Next));
    putchar('\n');
  }

  arenaAlloc(BaseArena, ARENA_PAGE);
  printf("Size: %zu\n", BaseArena->Next->Total);
  printf("Offset: %zu\n", BaseArena->Next->Offset);
  printf("Remains: %zu\n", REMAIN_OF(BaseArena->Next));
  putchar('\n');

  arenaAlloc(BaseArena, ARENA_PAGE + 1);
  printf("Size: %zu\n", BaseArena->Next->Next->Total);
  printf("Offset: %zu\n", BaseArena->Next->Next->Offset);
  printf("Remains: %zu\n", REMAIN_OF(BaseArena->Next->Next));
  putchar('\n');

  arenaFree(BaseArena);
}

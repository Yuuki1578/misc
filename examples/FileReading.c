#include "../Misc.h"
#include <stdio.h>
#include <stdlib.h>

char *readEntireFile(Arena *Allocator, const char *FilePath) {
  FILE *Handler = fopen(FilePath, "rb");
  if (!Handler)
    exit(1);

  char *Buffer = arenaAlloc(Allocator, BUFSIZ);
  size_t Offset = 0, size = BUFSIZ;

  while (Buffer) {
    size_t Readed;
    if ((Readed = fread(Buffer + Offset, 1, BUFSIZ - 1, Handler)) == 0)
      break;

    Offset += Readed;
    Buffer = arenaRealloc(Allocator, Buffer, size, size * 2);
    size *= 2;
  }

  fclose(Handler);
  return Buffer;
}

int main(void) {
  Arena *AllocatorContext = arenaCreate(ARENA_PAGE);
  char *FileContent = readEntireFile(AllocatorContext, __FILE__);

  printf("%s", FileContent);

  arenaAlloc(AllocatorContext, 4000);
  printf("Total size of arena: %zu\n"
         "Offset: %zu\n"
         "Remains: %zu\n",
         AllocatorContext->Next->Total, AllocatorContext->Next->Offset,
         REMAIN_OF(AllocatorContext->Next));

  arenaAlloc(AllocatorContext, 4000);
  printf("Total size of arena: %zu\n"
         "Offset: %zu\n"
         "Remains: %zu\n",
         AllocatorContext->Next->Next->Total,
         AllocatorContext->Next->Next->Offset,
         REMAIN_OF(AllocatorContext->Next->Next));

  arenaFree(AllocatorContext);
}

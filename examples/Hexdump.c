#define MISC_USE_GLOBAL_ALLOCATOR
#define VECTOR_ALLOC_FREQ (1024)
#include "../Misc.h"
#include <errno.h>

#if defined(__LP64__) || defined(__LLP64__)
#define PRINTABLE_SPAN (16)
#else
#define PRINTABLE_SPAN (8)
#endif

void visitByte(char *Content, size_t Length, size_t Span);
void modeStdin(size_t Span);

int main(int argc, char **argv) {
  ARENA_INIT();

  if (argc == 1) {
    modeStdin(PRINTABLE_SPAN);
  } else {
    char *FileContent = fileRead(argv[1]);
    if (FileContent == NULL) {
      fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
      ARENA_DROP();
      return 1;
    }

    visitByte(FileContent, strlen(FileContent) + 1, PRINTABLE_SPAN);
  }

  ARENA_DROP();
}

void visitByte(char *Content, size_t Length, size_t Span) {
  for (size_t I = 0, J = 0; I < Length; I++) {
    if (J != Span) {
      printf("%02X ", Content[I]);
      J++;
    } else {
      putchar('\n');
      J = 0;
    }
  }

  putchar('\n');
}

void modeStdin(size_t Span) {
  String Buffer = stringCreateWith(1024);
  int Character;

  while ((Character = fgetc(stdin)) != EOF)
    stringPush(&Buffer, Character);

  visitByte(vectorGet((void *)&Buffer, 0), Buffer.Buffer.Length + 1, Span);
}

#define MISC_USE_GLOBAL_ALLOCATOR
#include "../Misc.h"
#include <errno.h>

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;

  ARENA_INIT();
  char *FileContent = fileRead(argv[1]);

  if (FileContent == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    ARENA_DROP();
    return 1;
  }

  printf("%s", FileContent);
  ARENA_DROP();
}

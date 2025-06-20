#include <libmisc/file.h>
#include <stdlib.h>

int main(void) {
  char *buffer = FILERead(__FILE__);
  if (buffer == NULL)
    return 1;

  printf("%s\n", buffer);

  free(buffer);
  return 0;
}

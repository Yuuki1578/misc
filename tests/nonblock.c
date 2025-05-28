#include <libmisc/nonblock.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  char *buffer = "Hello, world!\n";
  size_t len = strlen(buffer);

  writenb(STDOUT_FILENO, buffer, len, 10);
}

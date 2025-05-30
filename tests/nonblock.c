#include <fcntl.h>
#include <libmisc/nonblock.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  int file = open("/data/data/com.termux/files/home/dev/misc/build/dump",
                  O_RDONLY | O_NONBLOCK | O_LARGEFILE);

  if (file == -1)
    return 1;

  char *buf = readnball(file, -1); // -1 timeout for unlimited timeout
  if (buf != nullptr) {
    writenb(STDOUT_FILENO, buf, strlen(buf), -1);
    free(buf);
  }

  close(file);
}

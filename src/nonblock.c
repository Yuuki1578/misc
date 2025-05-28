#include <libmisc/nonblock.h>
#include <stdlib.h>
#include <string.h>

static ssize_t __ionb(int nbfd, void *buf, size_t count, int event,
                      int timeout) {
  struct pollfd poller = {
      .fd = nbfd,
      .events = event,
  };

  ssize_t status;

  if (count == 0)
    return 0;

  while (true) {
    if ((status = poll(&poller, 1, timeout)) == 0)
      return status;
    else if (status == -1)
      return status;

    switch (event) {
    case POLLIN:
      if (poller.revents & event) {
        status = read(nbfd, buf, count);
      }

      goto endpoll;

    case POLLOUT:
      if (poller.revents & event) {
        status = write(nbfd, buf, count);
      }

      goto endpoll;

    default:
      // event not supported
      return -1;
    }
  }

endpoll:
  return status;
}

ssize_t readnb(int nbfd, void *buf, size_t count, int timeout) {
  return __ionb(nbfd, buf, count, POLLIN, timeout);
}

void *readnball(int nbfd, int timeout) {
  off64_t length = lseek64(nbfd, 0, SEEK_END);

  if (length == -1)
    return nullptr;
  else if (lseek64(nbfd, 0, SEEK_SET) == -1)
    return nullptr;

  char *buff = malloc(length);
  if (!buff)
    return nullptr;

  if (length <= MISCNB_PARTIAL)
    readnb(nbfd, buff, length, timeout);
  else {
    size_t count = 0;
    while (readnb(nbfd, buff + count, MISCNB_PARTIAL, timeout) > 0)
      count += MISCNB_PARTIAL;

    size_t len = strlen(buff) + 1;
    if (count > len) {
      char *tmp = realloc(buff, len);
      if (!tmp) {
        return buff;
      }

      buff = tmp;
    }
  }

  return buff;
}

ssize_t writenb(int nbfd, void *buf, size_t count, int timeout) {
  return __ionb(nbfd, buf, count, POLLOUT, timeout);
}

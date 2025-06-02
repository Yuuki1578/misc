#include <libmisc/arena.h>
#include <libmisc/nonblock.h>
#include <stdlib.h>
#include <string.h>

void pollreg_multiplex(Poll_Register *pr, On_Ready callback, void *any) {
  nfds_t done_io = 0;

  if (pr == nullptr || pr->count == 0)
    return;

  if (callback == nullptr)
    return;

  while (done_io != pr->count) {
    int status_poll = poll(pr->polls, pr->count, pr->timeout);

    if (status_poll == -1 || status_poll == 0)
      break;

    for (register nfds_t ind = 0; ind < pr->count; ind++) {
      if (pr->polls[ind].fd == -1)
        goto next_fd;

      if (pr->polls[ind].revents & pr->polls[ind].events) {
        switch (callback(pr->polls[ind].fd, pr->polls[ind].revents, any)) {
        case EVTRIG_EVENT_DONE:
          pr->polls[ind].fd = -1;
          done_io++;

        default:;
        }
      }
    next_fd:;
    }
  }
}

static ssize_t __ionb(int nbfd, void *buf, size_t count, int event,
                      Milisecond_t timeout) {
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

ssize_t readnb(int nbfd, void *buf, size_t count, Milisecond_t timeout) {
  return __ionb(nbfd, buf, count, POLLIN, timeout);
}

void *readnball(int nbfd, Milisecond_t timeout) {
  size_t readed = 0, buffer_cap = PAGE_SIZE;
  ssize_t from_readcall;
  char *buffer = calloc(buffer_cap, 1);

  if (buffer == nullptr)
    return nullptr;

  while (true) {
    if (readed >= buffer_cap - 1) {
      buffer_cap *= 2;
      char *tmp = realloc(buffer, buffer_cap);

      if (tmp == nullptr)
        return buffer;

      buffer = tmp;
    }

    if ((from_readcall =
             readnb(nbfd, buffer + readed, MISCNB_PARTIAL, timeout)) <= 0)
      break;

    readed += from_readcall;
  }

  if (readed < buffer_cap) {
    char *tmp = realloc(buffer, readed + 1);
    if (tmp == nullptr)
      return buffer;

    buffer = tmp;
  }

  return buffer;
}

ssize_t writenb(int nbfd, void *buf, size_t count, Milisecond_t timeout) {
  return __ionb(nbfd, buf, count, POLLOUT, timeout);
}

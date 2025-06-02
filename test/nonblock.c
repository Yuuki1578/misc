#include <fcntl.h>
#include <libmisc/nonblock.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

struct buf_t {
  char *buf;
  size_t len;
};

Event_Trigger on_call(int fd, int revents, void *any) {
  struct buf_t *buf = any;
  ssize_t readed = 0;

  if (revents != POLLIN)
    return EVTRIG_EVENT_DONE;

  while (true) {
    if ((readed = read(fd, buf->buf + buf->len, 32)) <= 0)
      break;

    buf->len += readed;
  }

  return EVTRIG_EVENT_DONE;
}

void pollreg_test(void) {
  int fd1 = open(__FILE__, O_RDONLY | O_NONBLOCK),
      fd2 = open("/data/data/com.termux/files/home/.bashrc",
                 O_RDONLY | O_NONBLOCK);

  struct buf_t buffer = {nullptr, 0};

  Poll_Register pr = {
      .polls = nullptr,
      .count = 2,
      .timeout = 1,
  };

  (void)posix_memalign((void **)&buffer.buf, 8, 1 << 12);
  (void)posix_memalign((void **)&pr.polls, 8, sizeof(struct pollfd) * 2);

  pr.polls[0] = (struct pollfd){
      .fd = fd1,
      .events = POLLIN,
  };

  pr.polls[1] = (struct pollfd){
      .fd = fd2,
      .events = POLLIN,
  };

  pollreg_multiplex(&pr, on_call, &buffer);
  // (void)write(STDOUT_FILENO, buffer.buf, buffer.len);

  close(fd1);
  close(fd2);

  free(buffer.buf);
  free(pr.polls);
}

int main(void) {
  pollreg_test();
  return 0;
}

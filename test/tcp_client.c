#include <libmisc/ipc/tcp.h>
#include <stddef.h>
#include <stdio.h>
#include <threads.h>

int Request(void *_) {
  // Create a connection to loopback address.
  TcpStream *stream = TcpStreamConnect(NULL, 8000);

  // Setup a buffer.
  char buf[(1 << 12) * 2];

  if (stream == NULL)
    return 1;

  // Setting up a timeout for @recv().
  TcpStreamSetTimeout(stream, 120);
  TcpStreamRecv(stream, buf, sizeof(buf) - 1, 0);
  TcpStreamShutdown(stream);

  printf("%s", buf);
  return 1;
}

int main(void) {
  int counter = 0;

  for (int i = 0, result; i < 255; i++, counter += result) {
    thrd_t thread;
    thrd_create(&thread, Request, NULL);
    thrd_join(thread, &result);
  }

  printf("Request count: %d\n", counter);

  return 0;
}

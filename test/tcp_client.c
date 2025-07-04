#include <libmisc/ipc/tcp.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <threads.h>

int Request(void *args) {
  // Create a connection to loopback address.
  TcpStream *stream = TcpStreamConnect(NULL, 8000);
  int       *result = args;

  // Setup a buffer.
  char buf[(1 << 12) * 2];

  if (stream == NULL) {
    printf("Nahh you don't\n");
    return 1;
  }

  thrd_yield();

  // Setting up a timeout for @recv().
  TcpStreamSetTimeout(stream, -1);
  if (TcpStreamRecvPartial(stream, buf, sizeof(buf) - 1, MSG_DONTWAIT) > 0)
    *result = 1;

  TcpStreamShutdown(stream, SHUT_RDWR);

  printf("%s", buf);
  return 0;
}

int main(void) {
  int counter = 0;

  for (int i = 0, result = 0; i < 255; i++, counter += result) {
    thrd_t thread;
    thrd_create(&thread, Request, &result);
    thrd_join(thread, NULL);
  }

  printf("Request count: %d\n", counter);

  return 0;
}

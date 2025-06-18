#include <libmisc/ipc/tcp.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
  // Create a connection to loopback address.
  TcpStream *stream = TcpStreamConnect(NULL, 8000);

  // Setup a buffer.
  char buf[(1 << 12) * 2];

  if (stream == NULL)
    return 1;

  // Setting up a timeout for @recv().
  TcpStreamSetTimeout(stream, 120);

  // Recieving bytes from loopback address.
  TcpStreamRecv(stream, buf, sizeof(buf) - 1, 0);
  printf("%s", buf);

  // Shutdown the stream.
  TcpStreamShutdown(stream);
  return 0;
}

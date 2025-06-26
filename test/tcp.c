#include <libmisc/ipc/tcp.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  // Setting up a listener and port.
  TcpListener *listener = TcpListenerNew("127.0.0.1", 8000);
  TcpStream   *stream   = NULL;

  if (listener == NULL)
    return 1;

  // Setup the backlog.
  if (TcpListenerListen(listener, 85) != 0) {
    TcpListenerShutdown(listener);
    return 2;
  }

  // Accepting connection for 65ms.
  while ((stream = TcpListenerAcceptFor(listener, 65)) != NULL) {
    char buffer[1 << 12] = {0};
    printf("Waiting\n");

    if (stream == STREAM_TIMED_OUT) {
      continue;
    }

    // Set timeout for both, @send() and @recv().
    TcpStreamSetTimeout(stream, 120);

    // Send the bytes.
    if (RECV(stream, buffer, sizeof(buffer) - 1) <= 0) {
      printf("Oh no\n");
      TcpStreamShutdown(stream, SHUT_RDWR);
      continue;
    }

    int sock = TcpStreamGetSocket(stream);

    shutdown(sock, SHUT_RD);
    SEND(stream, buffer, strlen(buffer));
    TcpStreamShutdown(stream, SHUT_WR);
  }

  // Shutting down the listener.
  TcpListenerShutdown(listener);
  return 0;
}

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
  if (TcpListenerListen(listener, 69) != 0) {
    TcpListenerShutdown(listener);
    return 2;
  }

  // Accepting connection for 65ms.
  while ((stream = TcpListenerAcceptFor(listener, 50)) != NULL) {
    char buffer[1 << 12] = {0};

    // Continue if timed out.
    if (stream == STREAM_TIMED_OUT) {
      continue;
    }

    printf("Connected\n");

    // Set timeout for both, @send and @recv.
    TcpStreamSetTimeout(stream, -1);

    // Recieve the bytes.
    if (RECV(stream, buffer, sizeof(buffer) - 1) <= 0) {

      // Kill the stream if it's fail to @recv and continue.
      TcpStreamDie(stream);
      continue;
    }

    // Shutdown only the @read ability.
    TcpStreamShutdown(stream, SHUT_RD);

    // Send back the recieved bytes to client.
    SEND(stream, buffer, strlen(buffer));

    // Kill the stream and we done, let's try again.
    TcpStreamDie(stream);
  }

  // Shutting down the listener.
  TcpListenerShutdown(listener);
  return 0;
}

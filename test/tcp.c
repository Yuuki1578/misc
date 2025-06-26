#include <libmisc/ipc/tcp.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>

// C23 only.
#if __STDC_VERSION__ >= 202300L
char buffer[] = {
#  embed "../src/tcp.c"
};
#else
char buffer[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from "
                "server, iyayyy!\n";
#endif

// Setting up a listener and port.
TcpListener *listener = NULL;
uint16_t     port     = 8000;

int main(void) {
  // Create a listener with loopback address.
  listener                   = TcpListenerNew(NULL, port);
  TcpStream *stream          = NULL;
  size_t     request_ignored = 0;
  if (listener == NULL)
    return 1;

  // Setup the backlog.
  if (TcpListenerListen(listener, 85) != 0) {
    TcpListenerShutdown(listener);
    return 2;
  }

  // Accepting connection for 65ms.
  while ((stream = TcpListenerAcceptFor(listener, 65)) != NULL) {
    if (stream == STREAM_TIMED_OUT) {
      if (request_ignored == SIZE_T_MAX)
        request_ignored = 0;

      printf("\rRequest timeout: %zu                     ", ++request_ignored);
      fflush(stdout);
      continue;
    }

    // Set timeout for both, @send() and @recv().
    TcpStreamSetTimeout(stream, 120);

    // Send the bytes.
    if (SEND(stream, buffer, sizeof(buffer) - 1) == -1) {
      TcpStreamShutdown(stream);
      continue;
    }

    // Shutting down the stream.
    TcpStreamShutdown(stream);
  }

  // Shutting down the listener.
  TcpListenerShutdown(listener);
  return 0;
}

#include <libmisc/ipc/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// C23 only.
#if __STDC_VERSION__ >= 202300L
char buffer[] = {
#  embed "../src/tcp.c"
};
#else
char buffer[] =
    "[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n["
    "RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n[RESPONSE]\n";
#endif

// Setting up a listener and port.
TcpListener *listener = NULL;
uint16_t     port     = 8000;

// Interrupt handler to cleaning up resources.
static void SigHandler(int signum) {
  printf("Caught signal SIGINT, cleaning up...\n");
  TcpListenerShutdown(listener);
  exit(signum);
}

int main(void) {
  // Create a listener with loopback address.
  listener          = TcpListenerNew(NULL, port);
  TcpStream *stream = NULL;

  // Handling the signal.
  signal(SIGINT, SigHandler);

  if (listener == NULL)
    return 1;

  // Setup the backlog.
  if (TcpListenerListen(listener, 1) != 0) {
    TcpListenerShutdown(listener);
    return 2;
  }

  // Accepting connection for 65ms.
  while ((stream = TcpListenerAcceptFor(listener, 65)) != NULL) {
    if (stream == STREAM_TIMED_OUT) {
      printf("\rNo Request, continue lalala... ");
      fflush(stdout);
      continue;
    }

    printf("\rCaught external request, yay!    ");
    fflush(stdout);

    // Set timeout for both, @send() and @recv().
    TcpStreamSetTimeout(stream, 120);

    // Send the bytes.
    if (TcpStreamSend(stream, buffer, sizeof(buffer) - 1, MSG_DONTWAIT) == -1) {
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

#include "libmisc/ipc/tcp.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// C23 only.
char Buffer[] = {
#embed "../src/tcp.c"
};

// Setting up a listener and port.
TcpListener *Listener = NULL;
uint16_t Port         = 8000;

// Interrupt handler to cleaning up resources.
void sighandler(int signum)
{
    printf("Caught signal SIGINT, cleaning up...\n");
    TcpListenerShutdown(Listener);
    exit(signum);
}

int main(void)
{
    // Create a listener with loopback address.
    Listener          = TcpListenerNew(NULL, Port);
    TcpStream *stream = NULL;

    // Handling the signal.
    signal(SIGINT, sighandler);

    if (Listener == NULL)
        return 1;

    // Setup the backlog.
    if (TcpListenerListen(Listener, 80) != 0) {
        TcpListenerShutdown(Listener);
        return 2;
    }

    // Accepting connection for 65ms.
    while ((stream = TcpListenerAcceptFor(Listener, 65)) != NULL) {
        if (stream == STREAM_TIMED_OUT)
            continue;

        // Set timeout for both, @send() and @recv().
        TcpStreamSetTimeout(stream, 120);

        // Send the bytes.
        if (TcpStreamSend(stream, Buffer, sizeof Buffer, 0) == -1) {
            TcpStreamShutdown(stream);
            continue;
        }

        // Shutting down the stream.
        TcpStreamShutdown(stream);
    }

    // Shutting down the listener.
    TcpListenerShutdown(Listener);
    return 0;
}

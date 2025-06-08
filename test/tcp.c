#include <libmisc/ipc/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char buffer[] = {
#embed "../src/tcp.c"
};

TcpListener *listener = NULL;
uint16_t port         = 8000;

void sighandler(int signum)
{
    printf("Caught signal SIGINT, cleaning up...\n");
    TcpListener_shutdown(listener);
    exit(signum);
}

int main(void)
{
    listener          = TcpListener_new(NULL, port);
    TcpStream *stream = NULL;

    signal(SIGINT, sighandler);

    if (listener == NULL)
        return 1;

    if (TcpListener_listen(listener, 10) != 0) {
        TcpListener_shutdown(listener);
        return 2;
    }

    while ((stream = TcpListener_accept(listener)) != NULL) {
        TcpStream_settimeout(stream, 95);

        if (TcpStream_send(stream, buffer, sizeof buffer, 0) == -1) {
            TcpStream_shutdown(stream);
            break;
        }

        TcpStream_shutdown(stream);
    }

    TcpListener_shutdown(listener);
    return 0;
}

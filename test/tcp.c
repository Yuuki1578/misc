#include <libmisc/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

char buffer[] = {
#embed "../src/tcp.c"
};

Tcp_Listener *listener;

void sighandler(int signum)
{
    printf("Caught signal SIGINT, cleaning up...\n");
    listener_shutdown(listener);
    exit(signum);
}

int main(void)
{
    listener           = listener_new(NULL, 8000);
    Tcp_Stream *stream = NULL;

    signal(SIGINT, sighandler);

    if (listener == NULL)
        return 1;

    if (listener_listen(listener, 10) != 0) {
        listener_shutdown(listener);
        return 2;
    }

    while ((stream = listener_accept(listener)) != NULL) {
        stream_settimeout(stream, 95);

        if (stream_send(stream, buffer, sizeof buffer, 0) == -1) {
            stream_shutdown(stream);
            break;
        }

        stream_shutdown(stream);
    }

    listener_shutdown(listener);
    return 0;
}

#include <libmisc/tcp.h>
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    Tcp_Stream *stream = stream_connect(NULL, 8000);
    char buf[(1 << 12) * 2];

    if (stream == NULL)
        return 1;

    stream_settimeout(stream, 120);
    stream_recv(stream, buf, sizeof(buf) - 1, 0);
    printf("%s", buf);

    stream_shutdown(stream);
    return 0;
}

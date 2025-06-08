#include <libmisc/ipc/tcp.h>
#include <stddef.h>
#include <stdio.h>

int main(void)
{
    TcpStream *stream = TcpStream_connect(NULL, 8000);
    char buf[(1 << 12) * 2];

    if (stream == NULL)
        return 1;

    TcpStream_settimeout(stream, 120);
    TcpStream_recv(stream, buf, sizeof(buf) - 1, 0);
    printf("%s", buf);

    TcpStream_shutdown(stream);
    return 0;
}

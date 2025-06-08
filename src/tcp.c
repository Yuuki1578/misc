#include <arpa/inet.h>
#include <libmisc/tcp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct Tcp_Listener {
    struct sockaddr_in tcp_addr;
    socklen_t tcp_len;
    int tcp_socket;
};

struct Tcp_Stream {
    struct sockaddr_in stream_addr;
    socklen_t stream_len;
    int stream_socket;
    int stream_timeout;
    unsigned char __pad[4];
};

Tcp_Listener *listener_new(const char *addr, uint16_t port)
{
    Tcp_Listener *listener = calloc(1, sizeof(struct Tcp_Listener));
    if (listener == NULL)
        return NULL;

    if ((listener->tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return NULL;

    listener->tcp_len  = sizeof(listener->tcp_addr);
    listener->tcp_addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_addr   = {0},
        .sin_port   = htons(port),
    };

    if (addr == NULL)
        addr = "127.0.0.1"; // loopback address

    if (!inet_pton(AF_INET, addr, &listener->tcp_addr.sin_addr)) {
        close(listener->tcp_socket);
        free(listener);
        return NULL;
    }

    struct sockaddr *sockaddr = (void *)&listener->tcp_addr;
    if (bind(listener->tcp_socket, sockaddr, listener->tcp_len) != 0) {
        close(listener->tcp_socket);
        free(listener);
        return NULL;
    }

    return listener;
}

int listener_listen(Tcp_Listener *listener, int backlog)
{
    if (listener == NULL)
        return -1;

    return listen(listener->tcp_socket, backlog);
}

Tcp_Stream *listener_accept(Tcp_Listener *listener)
{
    if (listener == NULL)
        return NULL;

    if (listener->tcp_socket == -1 || listener->tcp_len != sizeof(struct sockaddr_in))
        return NULL;

    Tcp_Stream *stream = calloc(1, sizeof(struct Tcp_Stream));
    if (stream == NULL)
        return NULL;

    struct sockaddr *addr  = (void *)&stream->stream_addr;
    stream->stream_timeout = 0;
    stream->stream_len     = listener->tcp_len;

    if ((stream->stream_socket = accept(listener->tcp_socket, addr, &stream->stream_len)) == -1) {
        free(stream);
        return NULL;
    }

    return stream;
}

void listener_shutdown(Tcp_Listener *listener)
{
    if (listener == NULL)
        return;

    close(listener->tcp_socket);
    free(listener);
}

Tcp_Stream *stream_connect(const char *addr, uint16_t port)
{
    Tcp_Stream *stream = calloc(1, sizeof(struct Tcp_Stream));
    if (stream == NULL)
        return NULL;

    if (addr == NULL)
        addr = "127.0.0.1";

    if ((stream->stream_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        free(stream);
        return NULL;
    }

    stream->stream_timeout = 0;
    stream->stream_len     = sizeof(struct sockaddr_in);
    stream->stream_addr    = (struct sockaddr_in){
           .sin_family = AF_INET,
           .sin_addr   = {0},
           .sin_port   = htons(port),
    };

    if (!inet_pton(AF_INET, addr, &stream->stream_addr.sin_addr)) {
        close(stream->stream_socket);
        free(stream);
        return NULL;
    }

    struct sockaddr *sockaddr = (void *)&stream->stream_addr;
    if (connect(stream->stream_socket, sockaddr, stream->stream_len) != 0) {
        close(stream->stream_socket);
        free(stream);
        return NULL;
    }

    return stream;
}

int stream_settimeout(Tcp_Stream *stream, int timeout_ms)
{
    if (stream == NULL)
        return -1;

    stream->stream_timeout = timeout_ms;
    return 0;
}

ssize_t stream_send(Tcp_Stream *stream, const void *buf, size_t count, int flags)
{
    // bytes sended
    ssize_t sended = 0;
    size_t remain  = count;

    if (stream == NULL)
        return -1;

    // don't poll() if timeout is 0
    if (stream->stream_timeout == 0)
        return send(stream->stream_socket, buf, count, flags);

    struct pollfd pfd = {
        .fd     = stream->stream_socket, // socket fd
        .events = POLLOUT,               // event for send(), sendto(), sendmsg()
    };

    for (; remain != 0;) {
        int pollstat = poll(&pfd, 1, stream->stream_timeout);
        if (pollstat == -1) // error
            return -1;

        if (pollstat == 0) // timeout before ready
            return 0;

        if (pfd.revents & POLLOUT) { // socket fd is ready
            if (count < BUFFER_FRAGMENT_SIZE)
                return send(pfd.fd, buf, count, flags);

            size_t chunk       = remain > BUFFER_FRAGMENT_SIZE ? BUFFER_FRAGMENT_SIZE : remain;
            ssize_t chunk_send = send(pfd.fd, buf + sended, chunk, flags);

            if (chunk_send == -1)
                return -1;
            else if (chunk_send == 0)
                break;

            sended += chunk_send;
            remain -= chunk_send;
        }
    }

    return sended;
}

ssize_t stream_recv(Tcp_Stream *stream, void *buf, size_t count, int flags)
{
    // bytes sended
    ssize_t recieved = 0;
    size_t remain    = count;

    if (stream == NULL)
        return -1;

    // don't poll() if timeout is 0
    if (stream->stream_timeout == 0)
        return recv(stream->stream_socket, buf, count, flags);

    struct pollfd pfd = {
        .fd     = stream->stream_socket, // socket fd
        .events = POLLIN,                // event for send(), sendto(), sendmsg()
    };

    for (; remain != 0;) {
        int pollstat = poll(&pfd, 1, stream->stream_timeout);
        if (pollstat == -1) // error
            return -1;

        if (pollstat == 0) // timeout before ready
            return 0;

        if (pfd.revents & POLLIN) { // socket fd is ready
            if (count < BUFFER_FRAGMENT_SIZE)
                return recv(pfd.fd, buf, count, flags);

            size_t chunk       = remain > BUFFER_FRAGMENT_SIZE ? BUFFER_FRAGMENT_SIZE : remain;
            ssize_t chunk_recv = recv(pfd.fd, buf + recieved, chunk, flags);

            if (chunk_recv == -1)
                return -1;
            else if (chunk_recv == 0)
                break;

            recieved += chunk_recv;
            remain -= chunk_recv;
        }
    }

    return recieved;
}

int stream_shutdown(Tcp_Stream *stream)
{
    if (stream == NULL)
        return -1;

    int status = shutdown(stream->stream_socket, SHUT_RDWR);
    free(stream);
    return status;
}

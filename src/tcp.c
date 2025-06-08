// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my responsibility at all.
//
// @file tcp.c
// @brief TCP/IPv4 API around UNIX socket

#include <arpa/inet.h>
#include <libmisc/ipc/tcp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct TcpListener {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int sockfd;
};

struct TcpStream {
    struct sockaddr_in addr;
    socklen_t addrlen;
    int sockfd;
    int timeout;
};

TcpListener *TcpListener_new(const char *addr, uint16_t port)
{
    TcpListener *listener = calloc(1, sizeof(struct TcpListener));
    if (listener == NULL)
        return NULL;

    if ((listener->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return NULL;

    listener->addrlen = sizeof(listener->addr);
    listener->addr    = (struct sockaddr_in){
           .sin_family = AF_INET,
           .sin_addr   = {0},
           .sin_port   = htons(port),
    };

    if (addr == NULL)
        addr = "127.0.0.1"; // loopback address

    if (!inet_pton(AF_INET, addr, &listener->addr.sin_addr)) {
        close(listener->sockfd);
        free(listener);
        return NULL;
    }

    struct sockaddr *sockaddr = (void *)&listener->addr;
    if (bind(listener->sockfd, sockaddr, listener->addrlen) != 0) {
        close(listener->sockfd);
        free(listener);
        return NULL;
    }

    return listener;
}

int TcpListener_listen(TcpListener *listener, int backlog)
{
    if (listener == NULL)
        return -1;

    return listen(listener->sockfd, backlog);
}

TcpStream *TcpListener_accept(TcpListener *listener)
{
    if (listener == NULL)
        return NULL;

    if (listener->sockfd == -1 || listener->addrlen != sizeof(struct sockaddr_in))
        return NULL;

    TcpStream *stream = calloc(1, sizeof(struct TcpStream));
    if (stream == NULL)
        return NULL;

    struct sockaddr *addr = (void *)&stream->addr;
    stream->timeout       = 0;
    stream->addrlen       = listener->addrlen;

    if ((stream->sockfd = accept(listener->sockfd, addr, &stream->addrlen)) == -1) {
        free(stream);
        return NULL;
    }

    return stream;
}

void TcpListener_shutdown(TcpListener *listener)
{
    if (listener == NULL)
        return;

    close(listener->sockfd);
    free(listener);
}

TcpStream *TcpStream_connect(const char *addr, uint16_t port)
{
    TcpStream *stream = calloc(1, sizeof(struct TcpStream));
    if (stream == NULL)
        return NULL;

    if (addr == NULL)
        addr = "127.0.0.1";

    if ((stream->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        free(stream);
        return NULL;
    }

    stream->timeout = 0;
    stream->addrlen = sizeof(struct sockaddr_in);
    stream->addr    = (struct sockaddr_in){
           .sin_family = AF_INET,
           .sin_addr   = {0},
           .sin_port   = htons(port),
    };

    if (!inet_pton(AF_INET, addr, &stream->addr.sin_addr)) {
        close(stream->sockfd);
        free(stream);
        return NULL;
    }

    struct sockaddr *sockaddr = (void *)&stream->addr;
    if (connect(stream->sockfd, sockaddr, stream->addrlen) != 0) {
        close(stream->sockfd);
        free(stream);
        return NULL;
    }

    return stream;
}

int TcpStream_sockfd(TcpStream *stream)
{
    if (stream == NULL)
        return -1;

    return stream->sockfd;
}

int TcpStream_settimeout(TcpStream *stream, int timeout_ms)
{
    if (stream == NULL)
        return -1;

    stream->timeout = timeout_ms;
    return 0;
}

ssize_t TcpStream_send(TcpStream *stream, const void *buf, size_t count, int flags)
{
    // bytes sended
    ssize_t sended = 0;
    size_t remain  = count;

    if (stream == NULL)
        return -1;

    // don't poll() if timeout is 0
    if (stream->timeout == 0)
        return send(stream->sockfd, buf, count, flags);

    struct pollfd pfd = {
        .fd     = stream->sockfd, // socket fd
        .events = POLLOUT,        // event for send(), sendto(), sendmsg()
    };

    for (; remain != 0;) {
        int pollstat = poll(&pfd, 1, stream->timeout);
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

ssize_t TcpStream_recv(TcpStream *stream, void *buf, size_t count, int flags)
{
    // bytes sended
    ssize_t recieved = 0;
    size_t remain    = count;

    if (stream == NULL)
        return -1;

    // don't poll() if timeout is 0
    if (stream->timeout == 0)
        return recv(stream->sockfd, buf, count, flags);

    struct pollfd pfd = {
        .fd     = stream->sockfd, // socket fd
        .events = POLLIN,         // event for recv(), recvfrom(), recvmsg()
    };

    for (; remain != 0;) {
        int pollstat = poll(&pfd, 1, stream->timeout);
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

int TcpStream_shutdown(TcpStream *stream)
{
    if (stream == NULL)
        return -1;

    int status = shutdown(stream->sockfd, SHUT_RDWR);
    free(stream);
    return status;
}

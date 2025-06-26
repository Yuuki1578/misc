// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my
// responsibility at all.
//
// @file tcp.c
// @brief TCP/IPv4 API around UNIX socket

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#ifndef MISC_TCP_UPCOMING
#  define MISC_TCP_UPCOMING
#endif

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <libmisc/ipc/tcp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct TcpListener {
  struct sockaddr_in addr;
  socklen_t          addrlen;
  int                sockfd;
};

struct TcpStream {
  struct sockaddr_in addr;
  socklen_t          addrlen;
  int                sockfd;
  int                timeout;
};

enum TcpStreamIOKind {
  TCP_GOING_OUT = POLLOUT,
  TCP_GOING_IN  = POLLIN,
};

TcpListener *TcpListenerNew(const char *addr, uint16_t port) {
  TcpListener     *listener = calloc(1, sizeof(struct TcpListener));
  struct sockaddr *sockaddr;
  int              default_flags;

  if (listener == NULL)
    return NULL;

  if ((listener->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    return NULL;

  if ((default_flags = fcntl(listener->sockfd, F_GETFL)) >= 0)
    (void)fcntl(listener->sockfd, F_SETFL, default_flags | O_NONBLOCK);

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

  sockaddr = (void *)&listener->addr;
  if (bind(listener->sockfd, sockaddr, listener->addrlen) != 0) {
    close(listener->sockfd);
    free(listener);
    return NULL;
  }

  return listener;
}

int TcpListenerListen(TcpListener *listener, int backlog) {
  if (listener == NULL)
    return -1;

  return listen(listener->sockfd, backlog);
}

TcpStream *TcpListenerAccept(TcpListener *listener) {
  return TcpListenerAcceptFor(listener, -1);
}

TcpStream *TcpListenerAcceptFor(TcpListener *listener, int timeout_ms) {
  TcpStream       *stream;
  struct sockaddr *addr;
  struct pollfd    pfd;
  int              default_flags;

  if (listener == NULL)
    return NULL;

  if (listener->sockfd == -1 || listener->addrlen != sizeof(struct sockaddr_in))
    return NULL;

  stream = calloc(1, sizeof(struct TcpStream));
  if (stream == NULL)
    return NULL;

  addr            = (void *)&stream->addr;
  pfd             = (struct pollfd){listener->sockfd, POLLIN, 0};
  stream->timeout = 0;
  stream->addrlen = listener->addrlen;

start_poll:
  switch (poll(&pfd, 1, timeout_ms)) {
  case -1:
    return NULL;

  case 0:
    return STREAM_TIMED_OUT;

  default:
    if (pfd.fd & POLLIN)
      if ((stream->sockfd = accept4(listener->sockfd, addr, &stream->addrlen,
                                    SOCK_NONBLOCK)) == -1) {
        switch (errno) {
        case EAGAIN:
          goto start_poll;
        }

        free(stream);
        return NULL;
      }
  }

  if ((default_flags = fcntl(stream->sockfd, F_GETFL)) < 0)
    return stream;

  (void)fcntl(stream->sockfd, F_SETFL, default_flags | O_NONBLOCK);
  return stream;
}

void TcpListenerShutdown(TcpListener *listener) {
  if (listener == NULL)
    return;

  close(listener->sockfd);
  free(listener);
}

TcpStream *TcpStreamConnect(const char *addr, uint16_t port) {
  TcpStream       *stream = calloc(1, sizeof(struct TcpStream));
  struct sockaddr *sockaddr;
  int              default_flags;

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

  sockaddr = (void *)&stream->addr;
  if (connect(stream->sockfd, sockaddr, stream->addrlen) != 0) {
    close(stream->sockfd);
    free(stream);
    return NULL;
  }

  if ((default_flags = fcntl(stream->sockfd, F_GETFL)) >= 0)
    (void)fcntl(stream->sockfd, F_SETFL, default_flags | O_NONBLOCK);

  return stream;
}

int TcpStreamGetSocket(TcpStream *stream) {
  if (stream == NULL)
    return -1;

  return stream->sockfd;
}

int TcpStreamSetTimeout(TcpStream *stream, int timeout_ms) {
  if (stream == NULL)
    return -1;

  stream->timeout = timeout_ms;
  return 0;
}

static ssize_t TcpStreamPartialIO(enum TcpStreamIOKind kind, TcpStream *stream,
                                  void *buf, size_t count, int flags) {
  ssize_t       result      = 0;
  size_t        remain      = count;
  struct pollfd stream_poll = {0};

  if (stream == NULL)
    return -1;

  if (stream->timeout == 0)
    return send(stream->sockfd, buf, count, flags);

  stream_poll.fd     = stream->sockfd;
  stream_poll.events = POLLOUT | POLLIN;

  for (; remain != 0;) {
    int     pollstat = -1;
    size_t  chunk    = 0;
    ssize_t chunk_io = 0;

    pollstat = poll(&stream_poll, 1, stream->timeout);
    if (pollstat == -1) // error
      return -1;

    if (pollstat == 0) // timeout before ready
      return 0;

    if (stream_poll.revents & kind) { // socket fd is ready
      if (count < BUFFER_FRAGMENT_SIZE) {
        if (kind == TCP_GOING_OUT)
          return send(stream_poll.fd, buf, count, flags);
        else
          return recv(stream_poll.fd, buf, count, flags);
      }

      chunk = remain > BUFFER_FRAGMENT_SIZE ? BUFFER_FRAGMENT_SIZE : remain;

      if (kind == TCP_GOING_OUT)
        chunk_io = send(stream_poll.fd, buf + result, chunk, flags);
      else
        chunk_io = recv(stream_poll.fd, buf + result, chunk, flags);

      if (chunk_io == -1)
        return -1;
      else if (chunk_io == 0)
        break;

      result += chunk_io;
      remain -= chunk_io;
    }
  }

  return result;
}

ssize_t TcpStreamSendPartial(TcpStream *stream, void *buf, size_t count,
                             int flags) {
  return TcpStreamPartialIO(TCP_GOING_OUT, stream, buf, count, flags);
}

ssize_t TcpStreamRecvPartial(TcpStream *stream, void *buf, size_t count,
                             int flags) {
  return TcpStreamPartialIO(TCP_GOING_IN, stream, buf, count, flags);
}

static ssize_t TcpStreamIO(enum TcpStreamIOKind kind, TcpStream *stream,
                           void *buf, size_t count, int flags) {
  struct pollfd stream_poll = {0};

  if (stream == NULL)
    return -1;

  if (stream->timeout == 0) {
    if (kind == TCP_GOING_OUT)
      return send(stream->sockfd, buf, count, 0);

    return recv(stream->sockfd, buf, count, 0);
  }

  stream_poll.fd     = stream->sockfd;
  stream_poll.events = kind == TCP_GOING_OUT ? POLLOUT : POLLIN;

  for (;;) {
    int poll_result = poll(&stream_poll, 1, stream->timeout);

    switch (poll_result) {
    case -1:
      return -1;

    case 0:
      return 0;
    }

    switch (kind) {
    case TCP_GOING_OUT:
      if (stream_poll.revents & stream_poll.events)
        return send(stream_poll.fd, buf, count, flags);
      else
        return -1;

    case TCP_GOING_IN:
      if (stream_poll.revents & stream_poll.events)
        return recv(stream_poll.fd, buf, count, flags);
      else
        return -1;
    }
  }
}

ssize_t TcpStreamSend(TcpStream *stream, void *buf, size_t count, int flags) {
  return TcpStreamIO(TCP_GOING_OUT, stream, buf, count, flags);
}

ssize_t TcpStreamRecv(TcpStream *stream, void *buf, size_t count, int flags) {
  return TcpStreamIO(TCP_GOING_IN, stream, buf, count, flags);
}

int TcpStreamShutdown(TcpStream *stream, int flags) {
  if (stream == NULL)
    return -1;

  int status = shutdown(stream->sockfd, flags);
  free(stream);
  return status;
}

ssize_t TcpStreamAcceptPool(TcpPool *pool, TcpListener *listener,
                            int timeout_ms, size_t max_conn) {
  if (pool == NULL || listener == NULL)
    return -1;

  if (pool->item_size != sizeof(TcpStream))
    return -1;

  if (max_conn == 0)
    return 0;

  for (size_t i = 0; i < max_conn; i++) {
    TcpStream *stream = TcpListenerAcceptFor(listener, timeout_ms);
    if (stream != NULL)
      VectorPush(pool, stream);

    free(stream);
  }

  return max_conn;
}

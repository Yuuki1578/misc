#pragma once

#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_FRAGMENT_SIZE (1024)

typedef struct Tcp_Listener Tcp_Listener;
typedef struct Tcp_Stream Tcp_Stream;

Tcp_Listener *listener_new(const char *addr, uint16_t port);
int listener_listen(Tcp_Listener *listener, int backlog);
Tcp_Stream *listener_accept(Tcp_Listener *listener);
void listener_shutdown(Tcp_Listener *listener);

Tcp_Stream *stream_connect(const char *addr, uint16_t port);
int stream_settimeout(Tcp_Stream *stream, int timeout_ms);
ssize_t stream_send(Tcp_Stream *stream, const void *buf, size_t count,
                    int flags);
ssize_t stream_recv(Tcp_Stream *stream, void *buf, size_t count, int flags);
int stream_shutdown(Tcp_Stream *stream);

// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my responsibility at all.
//
// @file tcp.h
// @brief TCP/IPv4 API around UNIX socket

#pragma once

#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

// Buffer count partial size.
// It's useful, because if the count for IO is too big,
// it might blocking the thread, so the IO API provide
// a way for setting up the timeout
#define BUFFER_FRAGMENT_SIZE (1024)

// TCP/IPv4 Listener server.
// The @Tcp_Listener struct is an opaque type for interacting
// with UNIX @IPPROTO_TCP server. Fill it with address and port,
// then you are ready to go.
typedef struct TcpListener TcpListener;

// TCP/IPv4 Connection stream.
// The @Tcp_Stream struct is also an opaque type for sending / recieving
// bytes between process using TCP/IPv4. This struct is returned from
// @listener_accept() or @stream_connect()
typedef struct TcpStream TcpStream;

// Create a newly allocated @Tcp_Listener struct with an address and port
// bound with it. @addr must be a valid IPv4 address.
//
// RETURN:
// return an allocated @Tcp_Listener on success, return NULL on failed and
// set @errno to corresponding error.
TcpListener *TcpListener_new(const char *addr, uint16_t port);

// Listening to incoming connections, with the maximum of @backlog's connections.
//
// RETURN:
// return 0 on success, -1 on error and set @errno.
int TcpListener_listen(TcpListener *listener, int backlog);

// Accepting a connection from @listener, returning a @Tcp_Stream instance
// for @stream_send() or @stream_recv().
//
// RETURN:
// return an allocated @Tcp_Stream on success, return NULL on error and set @errno.
TcpStream *TcpListener_accept(TcpListener *listener);

// Shutting down the server, freeing it's memory.
//
// RETURN:
// NONE
void TcpListener_shutdown(TcpListener *listener);

// Attempt to connect to a remote address from @addr in port @port.
//
// RETURN:
// return a pointer to @Tcp_Stream on success, return NULL on error and set @errno.
TcpStream *TcpStream_connect(const char *addr, uint16_t port);

// Return the underlying file descriptor for socket.
// The @listener can be a @TcpListener or a @TcpStream,
// (cast the type!).
//
// RETURN:
// return the valid file descriptor on success, return -1 on error.
int TcpStream_sockfd(TcpStream *stream);

// Setting up a timeout for @stream, limiting it's operation for @timeout_ms milisecond.
//
// RETURN:
// return 0 on sucess, return -1 on error.
int TcpStream_settimeout(TcpStream *stream, int timeout_ms);

// Sending a @count bytes @buffer to @stream.
// If the timeout is not set, this function might blocking the thread, if it's setted,
// then the @send() opeation is done by partialy sending a chunk of byte when ONLY the
// socket is ready to @send() or @POLLOUT.
//
// RETURN:
// return the total bytes sended to @stream on success, return -1 on error, return 0 on timeout.
ssize_t TcpStream_send(TcpStream *stream, const void *buf, size_t count, int flags);

// Recieving a @count bytes of data from @stream and save the data into @buf.
// The behavior is same as @stream_send(), it might or might not blocking the thread
// if the timeout is setted.
//
// RETURN:
// return the total bytes recieved from @stream on success, return -1 on error, return 0 on timeout.
ssize_t TcpStream_recv(TcpStream *stream, void *buf, size_t count, int flags);

// Shutting down the @stream, both for READ and WRITE.
//
// RETURN:
// return 0 on success, return -1 on error.
int TcpStream_shutdown(TcpStream *stream);

// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my
// responsibility at all.
//
// @file tcp.h
// @brief TCP/IPv4 API around UNIX socket

#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  error Windows is not supported
#endif

#include <libmisc/vector.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

// Buffer count partial size.
// It's useful, because if the count for IO is too big,
// it might blocking the thread, so the IO API provide
// a way for setting up the timeout
#define BUFFER_FRAGMENT_SIZE (1024)
#define STREAM_TIMED_OUT     ((void *)-1)

// Wrapper for @TcpStreamSend and @TcpStreamRecv with @MSG_DONTWAIT flag.
#if !defined(SEND) && !defined(RECV)
#  define SEND(stream, msg, msglen)                                            \
    TcpStreamSend(stream, msg, msglen, MSG_DONTWAIT)
#  define RECV(stream, buf, buflen)                                            \
    TcpStreamRecv(stream, buf, buflen, MSG_DONTWAIT)
#endif

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

// TCP/IPv4 Listener server.
// The @TcpListener struct is an opaque type for interacting
// with UNIX @IPPROTO_TCP server. Fill it with address and
// port, then you are ready to go.
typedef struct TcpListener TcpListener;

// TCP/IPv4 Connection stream.
// The @TcpStream struct is also an opaque type for sending
// / recieving bytes between process using TCP/IPv4. This
// struct is returned from
// @TcpListenerAcceptFor, @TcpListenerAccept or @TcpStreamConnect
typedef struct TcpStream TcpStream;

// Create a newly allocated @TcpListener struct with an
// address and port bound with it. @addr must be a valid
// IPv4 address.
//
// RETURN:
// return an allocated @TcpListener on success, return NULL
// on failed and set @errno to corresponding error.
TcpListener *TcpListenerNew(const char *addr, uint16_t port);

// Listening to incoming connections, with the maximum of
// @backlog's connections.
//
// RETURN:
// return 0 on success, -1 on error and set @errno.
int TcpListenerListen(TcpListener *listener, int backlog);

// Accepting a connection from @TcpListener, returning a
// @TcpStream instance for @TcpStreamSend or @TcpStreamRecv.
// WARNING: This function will block indefinitely. use
// @TcpStreamAcceptFor for nonblocking behavior without busy wait.
//
// RETURN:
// return an allocated @TcpStream on success, return NULL on
// error and set @errno.
TcpStream *TcpListenerAccept(TcpListener *listener);

// Accept a connection from @TcpListener for @timeout_ms
// miliseconds, negative @timeout_ms means block forever.
// This function returning 3 possible values.
//
// RETURN:
// 1. STREAM_TIMED_OUT: @accept call timed out after
// @timeout_ms miliseconds.
// 2. NULL: @accept call somehow fail.
// 3. A pointer to a @TcpStream instance on success.
TcpStream *TcpListenerAcceptFor(TcpListener *listener, int timeout_ms);

// Shutting down the @TcpListener, freeing it's memory.
//
// RETURN:
// NONE
void TcpListenerShutdown(TcpListener *listener);

// Attempt to connect to a remote address from @addr in port
// @port.
//
// RETURN:
// return a pointer to @TcpStream on success, return NULL on
// error and set @errno.
TcpStream *TcpStreamConnect(const char *addr, uint16_t port);

// Return the underlying file descriptor for socket.
// The @stream can be a @TcpListener or a @TcpStream,
// (cast the type!).
//
// RETURN:
// return the valid file descriptor on success, return -1 on error.
int TcpStreamGetSocket(TcpStream *stream);

// Setting up a timeout for @stream, limiting it's operation
// for @timeout_ms milisecond.
//
// RETURN:
// return 0 on sucess, return -1 on error.
int TcpStreamSetTimeout(TcpStream *stream, int timeout_ms);

// Sending a @count bytes @buf to a @stream.
// If the timeout is not set, this function might blocking
// the thread, if it set, then the @send operation is
// done by partialy sending a chunk of byte when ONLY the
// socket is ready to @send.
//
// RETURN:
// return the total bytes sended to a @stream on success,
// return -1 on error, return 0 on timeout.
ssize_t TcpStreamSendPartial(TcpStream *stream, void *buf, size_t count,
                             int flags);

// Sending a @count bytes @buf to a @stream immediately.
// Only @send the data whenever the socket is ready after @poll.
//
// RETURN:
// return the total bytes sended, return 0 on timeout, -1 on error.
ssize_t TcpStreamSend(TcpStream *stream, void *buf, size_t count, int flags);

// Recieving a @count bytes of data from @stream and save
// the data into @buf. The behavior is same as
// @TcpStreamSendPartial, it might or might not blocking the thread
// if the timeout is set.
//
// RETURN:
// return the total bytes recieved from @stream on success,
// return -1 on error, return 0 on timeout.
ssize_t TcpStreamRecvPartial(TcpStream *stream, void *buf, size_t count,
                             int flags);

// Recieving a @count bytes from a @stream and store it on @buf immediately.
// Only @recv the data whenever the socket is ready after @poll.
//
// RETURN:
// return the total bytes recieved, return 0 on timeout, -1 on error.
ssize_t TcpStreamRecv(TcpStream *stream, void *buf, size_t count, int flags);

// Shutting down the @stream using SHUT_RD, SHUT_WR or SHUT_RDWR.
//
// RETURN:
// return 0 on success, return -1 on error.
int TcpStreamShutdown(TcpStream *stream, int how);

// Shutting down and freeing the @stream, both for READ and WRITE.
//
// RETURN:
// NONE.
void TcpStreamDie(TcpStream *stream);

#ifdef __cplusplus
}
}
#endif

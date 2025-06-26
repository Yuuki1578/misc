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
// @listener_accept() or @stream_connect()
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

// Accepting a connection from @listener, returning a
// @TcpStream instance for @stream_send() or @stream_recv().
// WARNING: This function will block indefinitely. use
// @TcpStreamAcceptFor for nonblocking behavior without busy wait.
//
// RETURN:
// return an allocated @TcpStream on success, return NULL on
// error and set
// @errno.
TcpStream *TcpListenerAccept(TcpListener *listener);

// Accept a connection from @listener for @timeout_ms
// miliseconds, returning 3 possible values.
//
// RETURN:
// 1. STREAM_TIMED_OUT: @accept call timed out after
// @timeout_ms miliseconds.
// 2. NULL: @accept call somehow fail.
// 3. A pointer to @TcpStream instance on success.
TcpStream *TcpListenerAcceptFor(TcpListener *listener, int timeout_ms);

// Shutting down the server, freeing it's memory.
//
// RETURN:
// NONE
void TcpListenerShutdown(TcpListener *listener);

// Attempt to connect to a remote address from @addr in port
// @port.
//
// RETURN:
// return a pointer to @TcpStream on success, return NULL on
// error and set
// @errno.
TcpStream *TcpStreamConnect(const char *addr, uint16_t port);

// Return the underlying file descriptor for socket.
// The @listener can be a @TcpListener or a @TcpStream,
// (cast the type!).
//
// RETURN:
// return the valid file descriptor on success, return -1 on
// error.
int TcpStreamGetSocket(TcpStream *stream);

// Setting up a timeout for @stream, limiting it's operation
// for @timeout_ms milisecond.
//
// RETURN:
// return 0 on sucess, return -1 on error.
int TcpStreamSetTimeout(TcpStream *stream, int timeout_ms);

// Sending a @count bytes @buffer to @stream.
// If the timeout is not set, this function might blocking
// the thread, if it's set, then the @send() operation is
// done by partialy sending a chunk of byte when ONLY the
// socket is ready to @send() or @POLLOUT.
//
// RETURN:
// return the total bytes sended to @stream on success,
// return -1 on error, return 0 on timeout.
ssize_t TcpStreamSendPartial(TcpStream *stream, void *buf, size_t count,
                             int flags);

ssize_t TcpStreamSend(TcpStream *stream, void *buf, size_t count, int flags);

// Recieving a @count bytes of data from @stream and save
// the data into @buf. The behavior is same as
// @stream_send(), it might or might not blocking the thread
// if the timeout is set.
//
// RETURN:
// return the total bytes recieved from @stream on success,
// return -1 on error, return 0 on timeout.
ssize_t TcpStreamRecvPartial(TcpStream *stream, void *buf, size_t count,
                             int flags);

ssize_t TcpStreamRecv(TcpStream *stream, void *buf, size_t count, int flags);

// Shutting down the @stream, both for READ and WRITE.
//
// RETURN:
// return 0 on success, return -1 on error.
int TcpStreamShutdown(TcpStream *stream, int flags);

// WARNING: Not implemented yet!
#ifdef MISC_TCP_UPCOMING

ssize_t TcpStreamSendSpawn(TcpStream *stream, void *buf, size_t count,
                           int flags);

ssize_t TcpStreamRecvSpawn(TcpStream *stream, void *buf, size_t count,
                           int flags);

typedef Vector TcpPool;
typedef ssize_t (*AcceptHandler)(TcpStream *stream, void *buf, size_t count,
                                 int flags);
enum TcpStreamHandlerKind {
  TCP_MULTI_THREADED  = 0xfeed,
  TCP_SINGLE_THREADED = 0xface,
};

// Accept an unresolved connections and place it to a pool. ready to @poll().
//
// RETURN:
// Return the number of successfuly accepted connections on success, -1 on
// error.
ssize_t TcpStreamAcceptPool(TcpPool *pool, TcpListener *listener,
                            int timeout_ms, size_t max_conn);

ssize_t TcpStreamHandlePoolWith(enum TcpStreamHandlerKind kind, TcpPool *pool,
                                AcceptHandler handler, ssize_t accumulator);

ssize_t TcpStreamHandlePool(TcpPool *pool, AcceptHandler handler,
                            ssize_t *accumulator);
#endif
#ifdef __cplusplus
}
}
#endif

/*
 * April 2025, [https://github.com/Yuuki1578/misc.git]
 * This is a part of the libmisc library.
 * Any damage caused by this software is not my responsibility at all.

 * @file nonblock.h
 * @brief non-blocking I/O for file
 *
 * */

/*
 * The functionality itself is nonblock, but if the I/O count for such file
 * is too big, then the I/O operation might blocking the threads
 *
 * And there is solution for that fortunately, by using the function partialy,
 * we can minimalize the count for such byte stream from / into file descriptor
 *
 * */

/*
 * @SYNOPSIS
 *
 * @nbfd    -> nonblock file descriptor (open with O_NONBLOCK)
 * @buf     -> buffer for storing the file descriptor content
 * @count   -> how many bytes to store on @buf
 * @timeout -> timeout in milisecond, negative timeout means timeout forever
 *
 * */

#pragma once

#define _LARGEFILE64_SOURCE

#if defined(_WIN32) || defined(_WIN64)
#error Windows is not supported
#endif

#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>

enum { MISCNB_PARTIAL = 1 << 10 };

/*
 * Magic values
 *
 * */
typedef enum {
  EVTRIG_EVENT_DONE = 0xBADF00D,
  EVTRIG_EVENT_WAIT = 0xBADFEEL,
} EventTrigger;

/*
 * Timeout in milisecond
 *
 * */
typedef int milisecond_t;

/*
 * A Function pointer that get called when a file descriptor is ready
 * @SYNOPSIS
 *
 * @fd_context -> file descriptor that is ready
 * @fd_event   -> event from struct pollfd -> revents
 * @any        -> any data, can be converted into any pointer types
 *
 * */
typedef EventTrigger (*OnReady)(int fd_context, int fd_event, void *any);

/*
 * Data structure for poll_multiplex, member of this struct get passed to
 * poll() on the fly
 *
 * */
typedef struct {
  struct pollfd *polls;
  nfds_t count;
  milisecond_t timeout;
} PollRegister;

/*
 * Attempt to do an I/O operation or else, specified in @callback, on a ready
 * file descriptor, the @any parameter is passed onto callback when fd is ready
 *
 * This function checking in a loop, whether an fd is -1 or not, -1 indicate
 * that the @callback return @EVTRIG_EVENT_DONE, which mean the I/O operation is
 * done
 *
 * When there is no fd left, the function is return,
 *
 * The @oncall can be modified to return some event, the event is:
 * 1. @EVTRIG_EVENT_DONE -> Set the fd to -1 on return, I/O operation is done
 * 2. @EVTRIG_EVENT_WAIT -> The fd can be used again or the operation isn't done
 * yet
 *
 * */
void pollreg_multiplex(PollRegister *restrict pr, OnReady callback, void *any);

/*
 * Attempt to read from a file descriptor without blocking the main thread
 * The function may be use partialy
 *
 * The timeout is same as in poll(), negative timeout indicate unlimited timeout
 * The fd is going to be a member of a struct pollfd, as defined in poll.h,
 *
 * For reading, the events is POLLIN, while for writing is POLLOUT
 *
 * */
ssize_t readnb(int nbfd, void *buf, size_t count, milisecond_t timeout);

/*
 * Read all the file content until EOF
 * If the file length is greater than MISCNB_PARTIAL, the file is readed
 * partialy to prevent blocking the thread
 *
 * If not, read the file until EOF
 *
 * Is safe to use this function if your file size is less than or equal to 2^(64
 * - 1) - 1 bytes or approximately 9,2 GiB
 *
 * */
void *readnball(int nbfd, milisecond_t timeout);

/*
 * Write up to buf[count] to the file descriptor
 * The write operation is not synced using fsync(), so the user gotta do it
 * themselves
 *
 * */
ssize_t writenb(int nbfd, void *buf, size_t count, milisecond_t timeout);

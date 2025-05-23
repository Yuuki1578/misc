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

#pragma once

#define _LARGEFILE64_SOURCE

#if defined(_WIN32) || defined(_WIN64)
#error Windows is not supported
#endif

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>

/*
 * While these function provide nonblocking I/O, the file descriptor itself must be opened
 * with the O_NONBLOCK flags (fcntl.h)
 *
 * */
#define MISCNB_PARTIAL 1 << 10

enum MiscEvent {
    MISCNB_EVWRITE  = POLLWRNORM,
    MISCNB_EVREAD   = POLLRDNORM,
};

typedef struct {
    struct pollfd   poller;
    void            *buffer;
    size_t          count;
    bool            complete;
} Pollio;

/*
 * Registering file descriptor to be monitored with event
 * The event must be one of MISCNB_EV*
 *
 * This functionality is far differ from one defined in poll.h
 *
 * */
Pollio pollio_register(int fd, enum MiscEvent event);

/*
 * Monitoring each file descriptor, wait for it to be ready and perform reading
 * The readyness of file descriptor is still unknown, so the file descriptor
 * being read is may / may not be unordered
 *
 * */
void pollio_multiplex(Pollio *polls, size_t count, int timeout);

/*
 * Attempt to read from a file descriptor without blocking the main thread
 * The function may be use partialy
 * 
 * The timeout is same as in poll.h, minus timeout indicate unlimited timeout
 * The fd is going to be a member of a struct pollfd, as defined in poll.h,
 * 
 * For reading, the events is POLLRDNORM, while for writing is POLLWRNORM
 * I plan for adding more events such as POLLWRBAND, POLLRDBAND, etc
 *
 * */
ssize_t readnb(int      nbfd,
                void    *buf,
                size_t  count,
                int     timeout);

/*
 * Read all the file content until EOF
 * If the file length is greater than MISCNB_PARTIAL, the file is readed
 * partialy to prevent blocking the thread
 *
 * If not, read the file until EOF
 *
 * Is safe to use this function if your file size is less than or equal to 2^(64 - 1) - 1 bytes
 * or approximately 9,2 GiB
 *
 * */
void *readnball(int nbfd, int timeout);

/*
 * Write up to buf[count] to the file descriptor
 * The write operation is not synced using fsync(), so the user gotta do it themselves
 *
 * */
ssize_t writenb(int     nbfd,
                void    *buf,
                size_t  count,
                int     timeout);

#if !defined(_WIN32) || !defined(_WIN64)
#ifndef MISC_FS_H

#define MISC_FS_H
#define _FILE_OFFSET_BITS 64

#include <libmisc/arena.h>
#include <stdio.h>
#include <poll.h>

constexpr auto MISCIO_EVWRITE   = POLLWRNORM;
constexpr auto MISCIO_EVREAD    = POLLRDNORM;
constexpr auto MISCIO_PARTIAL   = BUFSIZ;
constexpr auto MISCIO_FDMAX     = 32768;

typedef nfds_t          pcount_t;
typedef struct pollfd   __poll_inner_t;

typedef struct {
    // EVWRITE: output
    // EVREAD:  input
    __poll_inner_t  inner;

    // EVWRITE: input
    // EVREAD:  output
    char            *buffer;
    size_t          count;
} PollFd;

extern char *fs_readall(Arena *arena, FILE *file);
extern char *fs_readall_fd(Arena *arena, int fd);

#ifdef MISC_NONBLOCK_UNLOCK

extern PollFd pfd_new(int fd, int event);
extern int fs_nb_open(const char *path, int flag, ...);
extern int fs_nb_read(Arena *arena, PollFd *pfds, pcount_t count);
extern int fs_nb_write(Arena *arena, PollFd, *pfds, pcount_t count);

#endif

#endif
#endif

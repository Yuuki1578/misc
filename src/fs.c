#if !defined(_WIN32) || !defined(_WIN64)

#include <libmisc/fs.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

char *fs_readall(Arena *arena, FILE *file)
{
    off_t offset;

    if (arena == nullptr || file == nullptr)
        return nullptr;
    
    clearerr(file);

    if (fseeko(file, 0, SEEK_END) != 0) {
        switch (errno) {
        case EOVERFLOW:
            offset = INT64_MAX;
            goto rewinded;
        }

        return nullptr;
    }

    else
        if ((offset = ftello(file)) <= 0)
            return nullptr;

rewinded:
    rewind(file);

    char *buffer = arena_alloc(arena, offset);
    if (buffer == nullptr)
        return nullptr;

    return fread(buffer, sizeof *buffer, offset, file) > 0 ?
        buffer : nullptr;
}

char *fs_readall_fd(Arena *arena, int fd)
{
    off_t offset;

    if (arena == nullptr)
        return nullptr;

    if ((offset = lseek(fd, 0, SEEK_END)) == EOF) {
        switch (errno) {
        case EOVERFLOW:
            offset = INT64_MAX;
            goto seek_zero;
        }

        return nullptr;
    }

seek_zero:
    if (lseek(fd, 0, SEEK_SET) == EOF)
        return nullptr;

    char *buffer = arena_alloc(arena, offset);
    if (buffer == nullptr)
        return nullptr;

    return read(fd, buffer, offset) != EOF ?
        buffer : nullptr;
}

PollFd pfd_new(int fd, int event)
{
    return (PollFd){
        .inner  = {
            .fd     = fd,
            .event  = event,
        },

        .buffer = nullptr,
        .count  = 0,
    };
}

PollFd pfd_write(int fd, char *buffer, size_t wrcount)
{
    PollFd pfd  = pfd_new(fd, MISCIO_EVWRITE);
    pfd.buffer  = buffer;
    pfd.count   = wrcount;

    return pfd;
}

PollFd pfd_read(Arena *arena, int fd, size_t rdcount)
{
    PollFd pfd  = pfd_new(fd, MISCIO_EVREAD);
    pfd.buffer  = arena_alloc(arena, count);
    pfd.count   = rdcount;

    return pfd;
}

int fs_nb_open(const char *path, int flag, ...)
{
    va_list va;
    int     fd;

    va_start(va, flag);
    fd = open(path, flag | O_NONBLOCK, va);

    va_end(va);
    return fd;
}

#endif

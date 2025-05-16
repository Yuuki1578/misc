#include <libmisc/fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

char *fs_readall(Arena *arena, FILE *file)
{
    off_t offset;

    if (arena == nullptr || file == nullptr)
        return nullptr;
    
    clearerr(file);

    if (fseeko(file, 0, SEEK_END) != 0)
        return nullptr;

    else
        if ((offset = ftello(file)) == EOF)
            return nullptr;

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

    if ((offset = lseek(fd, 0, SEEK_END)) == EOF)
        return nullptr;

    if (lseek(fd, 0, SEEK_SET) == EOF)
        return nullptr;

    char *buffer = arena_alloc(arena, offset);
    if (buffer == nullptr)
        return nullptr;

    return read(fd, buffer, offset) != EOF ?
        buffer : nullptr;
}

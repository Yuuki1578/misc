#define MISC_NONBLOCK_UNLOCK

#include <libmisc/arena.h>
#include <libmisc/fs.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    Arena *arena = &(Arena){};
    arena_new(arena, PAGE_SIZE, true);

    int fd = fs_nb_open("./include/libmisc/fs.h", O_RDONLY);
    char *content = fs_readall_fd(arena, fd);

    printf("%s", content);

    close(fd);
    arena_dealloc(arena);
}

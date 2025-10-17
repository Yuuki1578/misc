#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"
#include <errno.h>

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;

    ARENA_INIT();
    char *file_content = read_from_path(argv[1]);

    if (file_content == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        ARENA_DEINIT();
        return 1;
    }

    printf("%s", file_content);
    ARENA_DEINIT();
}

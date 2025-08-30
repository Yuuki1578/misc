#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"
#include <errno.h>

int main(int argc, char** argv)
{
    if (argc < 2)
        return 1;

    ARENA_INIT();
    char* content = file_read_all(argv[1]);

    if (content == NULL) {
        fprintf(stderr, "%s\n", strerror(errno));
        misc_free();
        return 1;
    }

    printf("%s", content);
    misc_free();
}

#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Arena* arena = arena_init(1 << 16);
    char* buf;

    buf = arena_alloc(arena, 64);
    memset(buf, 'A', 64);

    buf = arena_realloc(arena, buf, 64, 64 * 4);
    memset(buf + 64, 'B', 64 * 4);

    buf = arena_realloc(arena, buf, 64 * 4, 1 << 12);
    memset(buf + 64 * 4, 'C', (1 << 12) - 1);
    buf[(1 << 12) - 1] = 0;

    printf("%s\n", buf);
    printf("%zu\n", arena_size(arena));

    arena_free(arena);
}

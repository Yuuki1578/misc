#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Arena* arena = initArena(1 << 16);
    char* buf;

    for (usize i = 0; i < 1024; i++) {
        buf = cstrArenaPrintf(arena, "STRING IS: %zu:%zu:%zu\n", i, i * 2, i * 3);
        printf("%s", buf);
    }

    // buf = allocArena(arena, 64);
    // memset(buf, 'A', 64);

    // buf = reallocArena(arena, buf, 64, 64 * 4);
    // memset(buf + 64, 'B', 64 * 4);

    // buf = reallocArena(arena, buf, 64 * 4, 1 << 12);
    // memset(buf + 64 * 4, 'C', (1 << 12) - 1);
    // buf[(1 << 12) - 1] = 0;

    // for (usize i = 1; i <= 1 << 10; i++) {
    //     void* trash = allocArena(arena, i * 100);
    //     (void)trash;
    // }

    // printf("%s\n", buf);
    freeArena(arena);
}

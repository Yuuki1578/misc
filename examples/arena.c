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

    freeArena(arena);
}

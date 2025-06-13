#include "libmisc/arena.h"
#include <stdio.h>
#include <string.h>

Arena *arena = &(Arena){0};

int main(void)
{
    ArenaInit(arena, PAGE_SIZE, true);
    char *s = ArenaAlloc(arena, 512);

    if (s == NULL)
        goto cleanup;

    strcat(s, "Hello from Arena!\n");
    strcat(s, "Hello from Arena!\n");
    strcat(s, "Hello from Arena!\n");
    strcat(s, "Hello from Arena!\n");
    strcat(s, "Hello from Arena!\n");
    printf("%s", s);

cleanup:
    ArenaDealloc(arena);
}

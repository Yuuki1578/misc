#include "../misc.h"
#include <stdio.h>
#include <string.h>


int main(void)
{
    Arena *base_arena = arena_create(ARENA_PAGE);
    char *cstr = arena_alloc(base_arena, 32);
    strcpy(cstr, "Hello, world!");
    printf("%s\n", cstr);

    cstr = arena_realloc(base_arena, cstr, 32, 10);
    cstr[9] = '\0';
    printf("%s\n", cstr);

    char *large_buf = arena_alloc(base_arena, ARENA_PAGE * 2);
    if (large_buf != NULL) {
        memset(large_buf, 'E', ARENA_PAGE * 2 - 1);
        printf("%s\n", large_buf);
        printf("Size: %zu\n", base_arena->next->total);
        printf("Offset: %zu\n", base_arena->next->offset);
        printf("Remains: %zu\n", REMAIN_OF(base_arena->next));
        putchar('\n');
    }

    arena_alloc(base_arena, ARENA_PAGE);
    printf("Size: %zu\n", base_arena->next->total);
    printf("Offset: %zu\n", base_arena->next->offset);
    printf("Remains: %zu\n", REMAIN_OF(base_arena->next));
    putchar('\n');

    arena_alloc(base_arena, ARENA_PAGE + 1);
    printf("Size: %zu\n", base_arena->next->next->total);
    printf("Offset: %zu\n", base_arena->next->next->offset);
    printf("Remains: %zu\n", REMAIN_OF(base_arena->next->next));
    putchar('\n');

    arena_free(base_arena);
}

#include "../misc.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    Arena* base_arena = arena_create(ARENA_PAGE);
    char* str = arena_alloc(base_arena, 32);
    strcpy(str, "Hello, world!");
    printf("%s\n", str);

    str = arena_realloc(base_arena, str, 32, 10);
    str[9] = '\0';
    printf("%s\n", str);

    char* large = arena_alloc(base_arena, ARENA_PAGE * 2);
    if (large != NULL) {
        memset(large, 'E', ARENA_PAGE * 2 - 1);
        printf("%s\n", large);
        printf("Size: %zu\n", base_arena->next->total);
        printf("Offset: %zu\n", base_arena->next->offset);
        printf("Remains: %zu\n", remain_of(base_arena->next));
        putchar('\n');
    }

    arena_alloc(base_arena, ARENA_PAGE);
    printf("Size: %zu\n", base_arena->next->total);
    printf("Offset: %zu\n", base_arena->next->offset);
    printf("Remains: %zu\n", remain_of(base_arena->next));
    putchar('\n');

    arena_alloc(base_arena, ARENA_PAGE + 1);
    printf("Size: %zu\n", base_arena->next->next->total);
    printf("Offset: %zu\n", base_arena->next->next->offset);
    printf("Remains: %zu\n", remain_of(base_arena->next->next));
    putchar('\n');

    arena_free(base_arena);
}

#include "../include/libmisc/arena.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define lengthof(array) (sizeof array / sizeof *array)

static int numeric_table[] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
};

char* create_string_from_order(Arena* arena)
{
    char* base_str = arena_alloc(arena, 128);
    size_t offset = 0;

    for (size_t i = 0; i < lengthof(numeric_table); i++)
        for (int j = 0; j < numeric_table[i]; j++)
            base_str[offset++] = numeric_table[i] + '0';

    return base_str;
}

int main(void)
{
    Arena* context = arena_create(ARENA_PAGE);
    assert(context);

    printf("%s\n", create_string_from_order(context));

    arena_free(context);
}

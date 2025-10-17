#include "../misc.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#define LENGTH_OF(array) (sizeof(array) / sizeof *(array))

static int num_table[] = {
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

char *create_string_ord(Arena *allocator)
{
    char *buf = arena_alloc(allocator, 128);
    size_t offset = 0;

    for (size_t i = 0; i < LENGTH_OF(num_table); i++)
        for (int j = 0; j < num_table[i]; j++)
            buf[offset++] = num_table[i] + '0';

    return buf;
}

int main(void)
{
    Arena *context = arena_create(ARENA_PAGE);
    assert(context);

    printf("%s\n", create_string_ord(context));
    arena_free(context);
}

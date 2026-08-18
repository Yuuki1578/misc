#define MISC_IMPL
#include "../misc.h"
#define MAX 1024

u8 buffer[MAX];

int main(void)
{
    Fixed_Arena arena = {
        .buffer = buffer,
        .cap = sizeof buffer,
    };

    char* large_list[MAX];
    for (usize i = 0; i < MAX; i++) {
        large_list[i] = fa_alloc(&arena, sizeof(char));
        if (large_list[i] != NULL)
            *large_list[i] = 'A';

        printf("%.*s", 1, large_list[i]);
    }

    fa_clear(&arena);
}

// ./fixed_arena | wc -c == 1024

#define MISC_IMPL
#include "../misc.h"
#define MAX 1024

u8 buffer[MAX];

int main(void)
{
    char *large_list[MAX];
    struct memory_pool arena = {
        .buffer = buffer,
        .cap = sizeof buffer,
    };

    for (usize i = 0; i < MAX; i++) {
        large_list[i] = mp_alloc(&arena, sizeof(char));
        if (large_list[i] != NULL)
            *large_list[i] = 'A';

        printf("%.*s", 1, large_list[i]);
    }

    mp_clear(&arena);
}

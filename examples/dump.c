#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    arena_t *arena = arena_init(1 << 10);
    arena_alloc(arena, 17);

    int *k = arena_alloc(arena, sizeof *k);
    *k = 100;

    printf("%d\n", *k);
    
    arena_free(arena);
}

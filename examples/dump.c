#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    arena_t *arena = arena_create(1 << 5);
    struct allocator alloc = arena_as_allocator_with(libc_alloc, arena);
    int *ptr = alloc.alloc(alloc.any, sizeof *ptr, 4);

    *ptr = 69;
    printf("%d\n", *ptr);
}

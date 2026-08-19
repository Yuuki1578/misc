#define MISC_IMPL
#include "../misc.h"

void *alloc_from_arena(void *any, usize size, usize alignment)
{
    (void)alignment;
    return arena_alloc(any, size);
}

int main(void)
{
    char *buf;
    struct arena *arena = arena_init_with(misc_mmap_alloc, 1 << 16);
    struct allocator alloc = {
        .any = arena,
        .alloc = alloc_from_arena,
    };

    for (usize i = 0; i < 1024; i++) {
        buf = cstr_printf(&alloc, "STRING IS: %zu:%zu:%zu, hash = %lu\n", i, i * 2, i * 3, misc_fnv1a(&i, sizeof i));
        printf("%s", buf);
    }

    arena_free_with(misc_mmap_alloc, arena);
}

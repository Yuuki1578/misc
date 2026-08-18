#define MISC_IMPL
#include "../misc.h"

void* alloc_from_arena(void* any, usize size, usize alignment)
{
    Arena* arena = any;
    (void)alignment;
    return arena_alloc(arena, size);
}

int main(void)
{
    char* buf;
    Arena* arena = arena_init_with(misc_mmap_alloc, 1 << 16);
    Misc_Allocator alloc = {
        .any = arena,
        .allocate = alloc_from_arena,
    };

    for (usize i = 0; i < 1024; i++) {
        buf = cstr_printf(&alloc, "STRING IS: %zu:%zu:%zu\n", i, i * 2, i * 3);
        printf("%s", buf);
    }

    arena_free_with(misc_mmap_alloc, arena);
}

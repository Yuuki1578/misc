#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    void* alloc = misc_mmap_alloc;
    Arena* arena = arena_init_with(alloc, 1 << 16);
    char* buf;

    for (usize i = 0; i < 1024; i++) {
        buf = cstr_arena_printf(alloc, arena, "STRING IS: %zu:%zu:%zu\n", i, i * 2, i * 3);
        printf("%s", buf);
    }

    arena_free_with(alloc, arena);
}

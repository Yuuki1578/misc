#define MISC_IMPL
#include "../misc.h"

u8 stack_buffer[1024 + sizeof(Arena)];
u8 copy_buffer[1024 + sizeof(Arena)];

int main(void)
{
    Arena* arena = arena_init(sizeof stack_buffer, MISC_ARSTACK | MISC_ARNOGROW, stack_buffer);
    assert(arena != NULL);

    void* buf = arena_alloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 100, 128);

    buf = arena_alloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 200, 128);

    buf = arena_alloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 'A', 128);

    buf = arena_alloc(arena, 1024);
    assert(buf == NULL);

    arena_free(arena);
    assert(memcmp(stack_buffer, copy_buffer, sizeof stack_buffer) == 0);
}

#define MISC_IMPL
#include "../misc.h"

uint8_t stack_buffer[1024 + sizeof(arena_t)];
uint8_t copy_buffer[1024 + sizeof(arena_t)];

int main(void)
{
    arena_t* arena = arena_init(sizeof stack_buffer, MISC_ARSTACK | MISC_ARNOGROW, stack_buffer);
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

#include <libmisc/arena.h>

Arena *arena = &(Arena){0};

int main(void)
{
    arena_init(arena, PAGE_SIZE, true);

    arena_dealloc(arena);
}

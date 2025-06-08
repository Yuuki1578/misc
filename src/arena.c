// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my responsibility at all.
//
// @file arena.c
// @brief linear allocator with segmented region (arena)

#include <libmisc/arena.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

size_t Arena_remaining(Arena *arena) { return arena != NULL ? arena->capacity - arena->offset : 0; }

void *Arena_first_addr(Arena *arena) { return arena != NULL ? arena : NULL; }

void *Arena_last_addr(Arena *arena) { return arena != NULL ? arena->rawptr + arena->offset : NULL; }

void *Arena_brk_addr(Arena *arena)
{
    return arena != NULL ? arena->rawptr + arena->capacity : NULL;
}

bool Arena_on_limit(Arena *arena)
{
    if (arena == NULL)
        return false;

    if (arena->capacity - 1 <= arena->offset)
        return true;

    return false;
}

int Arena_init(Arena *arena, size_t step, bool should_allocate)
{
    if (arena == NULL)
        return ARENA_NOAVAIL;

    if (step == 0)
        return ARENA_NOAVAIL;

    if (should_allocate) {
        arena->rawptr = calloc(step, 1);

        if (arena->rawptr == NULL)
            return ARENA_NOAVAIL;
    }

    arena->capacity = step;
    arena->step     = step;
    arena->offset   = 0;
    return ARENA_READY;
}

void *Arena_alloc(Arena *arena, size_t size)
{
    void *ready;
    size_t remains;

    if (arena == NULL || size == 0)
        return NULL;

    remains = Arena_remaining(arena);

    if (arena->capacity == 0) {
        int status = Arena_init(arena, arena->step, true);
        if (status != ARENA_READY)
            return NULL;
    }

    // FIXME
    if (size >= arena->capacity || size >= remains) {
        size_t size_addition = size > arena->step ? size : arena->step;
        size_t half_remains  = (arena->capacity - remains) + size_addition;
        void *tmp            = realloc(arena->rawptr, half_remains);

        if (tmp == NULL)
            return NULL;

        arena->rawptr = tmp;
        arena->capacity += remains;
    }
    // FIXME

    ready = arena->rawptr + arena->offset;
    arena->offset += size;

    return ready;
}

void *Arena_realloc(Arena *arena, void *dst, size_t old_size, size_t new_size)
{
    void *ready;

    if (arena == NULL || old_size == 0)
        return NULL;

    if ((ready = Arena_alloc(arena, new_size)) == NULL)
        return NULL;

    if (dst == NULL)
        return ready;

    memcpy(ready, dst, old_size);
    return ready;
}

void Arena_dealloc(Arena *arena)
{
    if (arena == NULL || arena->capacity == 0)
        return;

    free(arena->rawptr);
    arena->capacity = 0;
    arena->offset   = 0;
}

void *Arena_popout(Arena *arena)
{
    if (arena == NULL)
        return NULL;

    void *mem = arena->rawptr;

    arena->rawptr   = NULL;
    arena->capacity = 0;
    arena->offset   = 0;
    arena->step     = 0;

    return mem;
}

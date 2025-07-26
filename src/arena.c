/* The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

#include "../include/libmisc/arena.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

bool arena_init(Arena *arena, size_t init_size, bool pre_alloc)
{
    if (arena == NULL || init_size == 0)
        return false;

    if (pre_alloc) {
        if ((arena->next_node = calloc(1, sizeof *arena)) == NULL)
            return false;
    } else {
        arena->next_node = NULL;
    }

    if ((arena->buffer = (uintptr_t)calloc(init_size, 1)) == 0) {
        if (pre_alloc)
            free(arena->next_node);

        return false;
    }

    arena->size   = init_size;
    arena->offset = 0;
    return true;
}

static Arena *arena_pull_last(Arena *base)
{
    register Arena *last = base;

    if (last == NULL)
        return NULL;

    while (true) {
        if (last->next_node != NULL)
            last = last->next_node;
        else
            break;
    }

    return last;
}

void *arena_alloc(Arena *arena, size_t size)
{
    Arena *last   = arena_pull_last(arena);
    void  *result = NULL;

    if (last == NULL || size < 1)
        return NULL;

    if (last->size == 0) {
        if (!arena_init(last, size, false))
            return NULL;

    } else if (last->size - last->offset < size) {
        size_t base_value = size >= last->size ? size : last->size;

        if ((last = arena_pull_last(last)) == NULL)
            return NULL;

        if (!arena_init(last, base_value * 2, true))
            return NULL;
    }

    result = (void *)(last->buffer + last->offset);
    last->offset += size;
    return result;
}

void *arena_realloc(Arena *arena, void *dst, size_t old_size, size_t new_size)
{
    void  *result       = NULL;
    size_t bytes_copied = 0;

    if (arena == NULL)
        return NULL;

    if (old_size == new_size)
        return (void *)dst;

    if (dst == NULL)
        return arena_alloc(arena, new_size);

    if ((result = arena_alloc(arena, new_size)) == NULL)
        return NULL;

    bytes_copied = old_size > new_size ? new_size : old_size;
    memcpy(result, dst, bytes_copied);
    return result;
}

void arena_free(Arena *arena)
{
    for (Arena *current = arena; current != NULL;) {
        if (current->size > 0)
            free((void *)current->buffer);

        if (current->next_node != NULL)
            current = current->next_node;
        else
            break;
    }
}

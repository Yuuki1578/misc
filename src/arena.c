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
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define SELF_PTR ((void*)69)

struct Arena {
    Arena *prev, *next;
    size_t offset, size;
    uint8_t *data;
};

Arena *arena_new(size_t size)
{
    Arena *arena = calloc(1, sizeof(Arena) + size);
    if (!arena)
        return NULL;

    arena->prev = NULL;
    arena->next = NULL;
    arena->offset = 0;
    arena->size = size;
    arena->data = (uint8_t*) arena + offsetof(Arena, data);

    return arena;
}

static inline Arena *find_suitable(Arena *arena, size_t spec)
{
    while (arena) {
        if (arena->size - arena->offset >= spec)
            return arena;
    }
    return NULL;
}

static inline Arena *get_last_node(Arena *arena)
{
    Arena *iter = arena;

    if (!iter)
        return NULL;

    while (iter->next)
        iter = iter->next;

    if (iter == arena)
        return SELF_PTR;

    return iter;
}

void *arena_alloc(Arena *arena, size_t size)
{
    if (!arena || size < 1)
        return NULL;

    Arena
        *suitable = find_suitable(arena, size),
        *last = get_last_node(arena),
        *used_arena = NULL;

    if (!suitable) {
        if (last == SELF_PTR) {
            Arena *new_instance = arena_new(size + ARENA_PAGE);
            new_instance->prev = arena;
            arena->next = new_instance;
            used_arena = new_instance;
        } else {
            
        }
    }
}

void arena_free(Arena *arena)
{
    Arena *save = arena;
    
    while (save != NULL) {
        Arena *prev = save->prev;
        free(save);
        save = prev;
    }
}

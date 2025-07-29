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

#define NEEDED(arena) ((arena)->size - (arena)->offset)

static Arena* create_child_node(Arena* head, size_t size)
{
    uint8_t* tmp = calloc(sizeof(Arena) + size, 1);
    uint8_t* data = tmp + offsetof(Arena, data);

    Arena* maybe_child = (void*)tmp;
    maybe_child->size = size;
    maybe_child->offset = 0;
    maybe_child->data = data;

    if (head == NULL)
        head = maybe_child;
    else {
        register Arena* current = head;
        while (current->next_node != NULL)
            current = current->next_node;

        current->next_node = maybe_child;
    }

    return head;
}

Arena* arena_create(size_t size)
{
    return create_child_node(NULL, size);
}

void* arena_alloc(Arena** arena, size_t size)
{
    Arena* current;

    if (arena == NULL || size < 1)
        return NULL;

    current = *arena;
    while (current != NULL) {
        if (NEEDED(current) >= size) {
            void* result = current->data + current->offset;
            current->offset += size;
            return result;
        }
        current = current->next_node;
    }

    size_t new_size = size > ARENA_PAGE ? size + ARENA_PAGE : ARENA_PAGE;
    Arena* new_arena = arena_create(new_size);

    new_arena->next_node = *arena;
    *arena = new_arena;

    void* result = new_arena->data + new_arena->offset;
    new_arena->offset += size;
    return result;
}

void arena_free(Arena* arena)
{
    while (arena != NULL) {
        Arena* next = arena->next_node;
        free(arena);
        arena = next;
    }
}

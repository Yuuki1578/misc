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
#include <string.h>

Arena* arena_create(size_t size)
{
    Arena* head = calloc(1, sizeof *head + size);
    if (!head)
        return NULL;

    head->next = NULL;
    head->total = size;
    head->offset = 0;
    head->data = ((uint8_t*)head) + sizeof *head;
    return head;
}

static Arena* find_suitable_arena(Arena* base, size_t size, int* found)
{
    Arena *iter = base, *last_nonnull;
    while (iter) {
        if (remainof(iter) >= size) {
            *found = 1;
            return iter;
        }

        last_nonnull = iter;
        iter = iter->next;
    }

    *found = 0;
    return last_nonnull;
}

void* arena_alloc(Arena* arena, size_t size)
{
    Arena* suitable;
    int found;
    size_t size_required;

    if (!arena || !size)
        return NULL;

    size_required = size >= ARENA_PAGE ? size : ARENA_PAGE;
    suitable = find_suitable_arena(arena, size, &found);

    if (!found) {
        suitable->next = arena_create(size_required);
        if (!suitable->next)
            return NULL;

        suitable = suitable->next;
    }

    void* result = suitable->data + suitable->offset;
    suitable->offset += size;
    return result;
}

void* arena_realloc(Arena* base, void* dst, size_t old_size, size_t new_size)
{
    void* result = arena_alloc(base, new_size);
    if (!result)
        return NULL;

    else if (!dst)
        return result;

    memcpy(result, dst, old_size > new_size ? new_size : old_size);
    return result;
}

void arena_free(Arena* base)
{
    while (base) {
        Arena* tmp = base->next;
        free(base);
        base = tmp;
    }
}

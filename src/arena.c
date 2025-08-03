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

    suitable = find_suitable_arena(arena, size, &found);
    size_required = size > suitable->total ? size * 2 : suitable->total * 2;

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

#include <libmisc/arena.h>
#include <stdlib.h>

bool arena_create(Arena *arena,
                  size_t init_size)
{
    if (arena == NULL || init_size == 0)
        return false;

    arena->child_node = NULL;
    arena->buffer = (uintptr_t) calloc(init_size, 1);

    if (arena->buffer == 0)
        return false;

    arena->size = init_size;
    arena->offset = 0;
    return true;
}

static bool arena_add_child(Arena *arena)
{
    Arena *child = arena->child_node;

    if (arena == NULL || arena->size == 0)
        return false;
    else if (child != NULL)
        return false;

    if ((child = calloc(1, sizeof *child)) == NULL)
        return false;

    return arena_create(child, arena->size);
}

void *arena_alloc(Arena *arena, size_t size);
void *arena_realloc(Arena *arana, size_t old_size, size_t new_size);

void arena_free(Arena *arena)
{
    for (Arena *current = arena; current != NULL;) {
        Arena *tmp = NULL;
        if (current->child_node != NULL) {
            tmp = current->child_node;
            if (current->buffer != 0)
                free((void *) current->buffer);

            free(current);
            current = tmp;
        } else {
            break;
        }
    }
}

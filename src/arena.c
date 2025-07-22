#include "libmisc/arena.h"
#include <stdlib.h>

bool arena_create(Arena *arena, size_t init_size) {
  if (arena == NULL || init_size == 0)
    return false;

  arena->parent     = arena;
  arena->child_node = NULL;
  arena->buffer     = (uintptr_t)calloc(init_size, 1);

  if (arena->buffer == 0)
    return false;

  arena->size   = init_size;
  arena->offset = 0;
  return true;
}

static bool arena_add_child(Arena *arena, size_t inherit) {
  Arena *child = arena->child_node;

  if (arena == NULL || arena->size == 0)
    return false;

  else if (child != NULL)
    return false;

  if ((child = calloc(1, sizeof *child)) == NULL)
    return false;

  return arena_create(child, inherit == 0 ? arena->size : inherit);
}

static Arena *arena_pull_last(Arena *parent) {
  Arena *current = parent;

  if (parent == NULL)
    return NULL;

  while (current->child_node != NULL)
    current = current->child_node;

  return current;
}

static inline size_t arena_remaining(Arena *arena) {
  return arena->size - arena->offset;
}

void *arena_alloc(Arena *arena, size_t size) {
  Arena *current_allocator = NULL;
  Arena *last              = NULL;
  void  *result            = NULL;

  if (arena == NULL || size == 0)
    return NULL;

  if ((last = arena_pull_last(arena)) == NULL)
    return NULL;

  if (arena_remaining(arena) <= size) {
    if (!arena_add_child(last, size + ARENA_LONG_PTR_PADDING))
      return false;
    current_allocator = last->child_node;
  } else {
    current_allocator = last;
  }

  result = (void *)(current_allocator->buffer + current_allocator->offset);
  current_allocator->offset += size;
  return result;
}

void *arena_realloc(Arena *arana, size_t old_size, size_t new_size);

void arena_free(Arena *arena) {
  for (Arena *current = arena; current != NULL;) {
    if (current->size > 0)
      free((void *)current->buffer);

    if (current->child_node != NULL)
      current = current->child_node;
    else
      break;
  }
}

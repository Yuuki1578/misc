#include "libmisc/arena.h"
#include <stdlib.h>

bool arena_create(Arena *arena, size_t init_size, bool create_child) {
  if (arena == NULL || init_size == 0)
    return false;

  if (create_child) {
    if ((arena->child_node = calloc(1, sizeof *arena)) == NULL)
      return false;
  } else {
    arena->child_node = NULL;
  }

  if ((arena->buffer = (uintptr_t)calloc(init_size, 1)) == 0) {
    free(arena->child_node);
    return false;
  }

  arena->size   = init_size;
  arena->offset = 0;
  return true;
}

static Arena *arena_pull_last(Arena *base) {
  register Arena *last = base;
  if (last == NULL)
    return NULL;

  while (true) {
    if (last->child_node != NULL)
      last = last->child_node;
    else
      break;
  }

  return last;
}

void *arena_alloc(Arena *arena, size_t size) {
  Arena *last   = arena_pull_last(arena);
  void  *result = NULL;

  if (last == NULL)
    return NULL;

  if (last->size == 0) {
    if (!arena_create(last, size, false))
      return NULL;

  } else if (last->size - last->offset <= size) {
    if ((last = arena_pull_last(last)) == NULL)
      return NULL;

    if (!arena_create(last, size, true))
      return NULL;
  }

  result = (void *)(last->buffer + last->offset);
  last->offset += size;
  return result;
}

void *arena_realloc(Arena *arena, void *dst, size_t old_size, size_t new_size) {
  void  *result       = NULL;
  size_t bytes_copied = 0;

  if (arena == NULL)
    return NULL;

  if (old_size == new_size)
    return dst;

  if (dst == NULL)
    return arena_alloc(arena, new_size);

  if ((result = arena_alloc(arena, new_size)) == NULL)
    return NULL;

  bytes_copied = old_size > new_size ? new_size : old_size;
  memcpy(result, dst, bytes_copied);
  return result;
}

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

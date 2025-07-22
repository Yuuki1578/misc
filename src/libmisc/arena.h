#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Arena {
  struct Arena *child_node;
  uintptr_t     buffer;
  size_t        size;
  size_t        offset;
} Arena;

bool  arena_create(Arena *arena, size_t init_size, bool create_child);
void *arena_alloc(Arena *arena, size_t size);
void *arena_realloc(Arena *arana, void *dst, size_t old_size, size_t new_size);
void  arena_free(Arena *arena);

#endif

// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my
// responsibility at all.
//
// @file arena.c
// @brief linear allocator with segmented region (arena)

#include <libmisc/arena.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct Arena {
  void  *rawptr;   // pointer to allocated memory.
  size_t capacity; // total memory that arena can hold.
  size_t offset;   // an offset from the left of the pointer.
  size_t step;     // how much bytes per allocation.
};

size_t ArenaRemaining(Arena *arena) {
  if (arena == NULL || arena->capacity == 0)
    return 0;

  return arena->capacity - arena->offset;
}

size_t ArenaOffset(Arena *arena) {
  if (arena == NULL)
    return 0;

  return arena->offset;
}

size_t ArenaCapacity(Arena *arena) {
  if (arena == NULL)
    return 0;

  return arena->capacity;
}

bool ArenaIsFull(Arena *arena) {
  if (arena == NULL)
    return false;

  if (arena->capacity - 1 <= arena->offset)
    return true;

  return false;
}

bool ArenaInit(Arena **arena, size_t step, bool should_allocate) {
  if (arena == NULL)
    return false;

  if (step == 0)
    return false;

  *arena = calloc(1, sizeof(Arena));
  if (*arena == NULL)
    return false;

  if (should_allocate) {
    (*arena)->rawptr = calloc(step, 1);

    if ((*arena)->rawptr == NULL)
      return false;
  }

  (*arena)->capacity = step;
  (*arena)->step     = step;
  (*arena)->offset   = 0;
  return true;
}

Arena *ArenaNew(size_t step, bool should_allocate) {
  Arena *arena;
  if (!ArenaInit(&arena, step, should_allocate))
    return NULL;

  return arena;
}

static inline size_t ExclusiveAdd(size_t rhs, size_t lhs) {
  if (rhs >= lhs)
    return rhs + lhs;

  // WARN: I DON'T KNOW ANY OF THIS
  return rhs + (lhs / rhs) * rhs + (lhs % rhs != 0 ? rhs : 0);
}

void *ArenaAlloc(Arena *arena, size_t size) {
  void  *ready;
  size_t remains;

  if (arena == NULL || size == 0)
    return NULL;

  remains = ArenaRemaining(arena);

  if (arena->capacity == 0) {
    if (!ArenaInit(&arena, arena->step, true))
      return NULL;
  }

  if (size >= remains) {
    if (size < arena->step)
      size = arena->step;

    size_t new_size = ExclusiveAdd(arena->capacity, size);
    void  *tmp      = realloc(arena->rawptr, new_size);
    if (tmp == NULL)
      return NULL;

    arena->rawptr   = tmp;
    arena->capacity = new_size;
  }

  ready = arena->rawptr + arena->offset;
  arena->offset += size;

  return ready;
}

void *ArenaRealloc(Arena *arena, void *dst, size_t old_size, size_t new_size) {
  void *ready;

  if (arena == NULL || old_size == 0)
    return NULL;

  if ((ready = ArenaAlloc(arena, new_size)) == NULL)
    return NULL;

  if (dst == NULL)
    return ready;

  memcpy(ready, dst, old_size);
  return ready;
}

void ArenaDealloc(Arena *arena) {
  if (arena == NULL || arena->capacity == 0)
    return;

  free(arena->rawptr);
  free(arena);
}

void *ArenaPopOut(Arena *arena) {
  if (arena == NULL)
    return NULL;

  return arena->rawptr;
}

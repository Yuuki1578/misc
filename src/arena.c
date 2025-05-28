/*
 * April 2025, [https://github.com/Yuuki1578/misc.git]
 * This is a part of the libmisc library.
 * Any damage caused by this software is not my responsibility at all.

 * @file arena.c
 * @brief linear allocator with segmented region (arena)
 *
 * */

#include <libmisc/arena.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

size_t arena_remaining(Arena *restrict arena) {
  return arena != nullptr ? arena->capacity - arena->offset : 0;
}

void *arena_first_addr(Arena *restrict arena) {
  return arena != nullptr ? arena : nullptr;
}

void *arena_last_addr(Arena *restrict arena) {
  return arena != nullptr ? arena->rawptr + arena->offset : nullptr;
}

void *arena_brk_addr(Arena *restrict arena) {
  return arena != nullptr ? arena->rawptr + arena->capacity : nullptr;
}

bool arena_on_limit(Arena *restrict arena) {
  if (arena == nullptr)
    return false;

  if (arena->capacity - 1 <= arena->offset)
    return true;

  return false;
}

int arena_init(Arena *restrict arena, size_t step, bool should_allocate) {
  if (arena == nullptr)
    return ARENA_NOAVAIL;

  if (step == 0)
    return ARENA_NOAVAIL;

  if (should_allocate) {
    arena->rawptr = calloc(step, 1);

    if (arena->rawptr == nullptr)
      return ARENA_NOAVAIL;
  }

  arena->capacity = step;
  arena->step = step;
  arena->offset = 0;
  return ARENA_READY;
}

void *arena_alloc(Arena *restrict arena, size_t size) {
  void *ready;
  size_t remains;

  if (arena == nullptr || size == 0)
    return nullptr;

  remains = arena_remaining(arena);

  if (arena->capacity == 0) {
    int status = arena_init(arena, arena->step, true);
    if (status != ARENA_READY)
      return nullptr;
  }

  if (size >= arena->capacity || size >= remains) {
    size_t size_addition = size > arena->step ? size : arena->step;
    size_t half_remains = (arena->capacity - remains) + size_addition;
    void *tmp = realloc(arena->rawptr, half_remains);

    if (tmp == nullptr)
      return nullptr;

    arena->rawptr = tmp;
    arena->capacity += remains;
  }

  ready = arena->rawptr + arena->offset;
  arena->offset += size;

  return ready;
}

void *arena_realloc(Arena *restrict arena, void *dst, size_t old_size,
                    size_t new_size) {
  void *ready;

  if (arena == nullptr || old_size == 0)
    return nullptr;

  if ((ready = arena_alloc(arena, new_size)) == nullptr)
    return nullptr;

  if (dst == nullptr)
    return ready;

  memcpy(ready, dst, old_size);
  return ready;
}

void arena_dealloc(Arena *restrict arena) {
  if (arena == nullptr || arena->capacity == 0)
    return;

  free(arena->rawptr);
  arena->capacity = 0;
  arena->offset = 0;
}

void *arena_popout(Arena *restrict arena) {
  if (arena == nullptr)
    return nullptr;

  void *mem = arena->rawptr;

  arena->rawptr = nullptr;
  arena->capacity = 0;
  arena->offset = 0;
  arena->step = 0;

  return mem;
}

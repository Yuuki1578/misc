// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my
// responsibility at all.
//
// @file arena.h
// @brief linear allocator with segmented region (arena)

// I don't know if bits/size.h exist on all UNIX like system
// so yeah. This device is using __ANDROID_API__ 24.

#pragma once

#define FIXED_PAGING_SIZE 4096

#if __STDC_VERSION__ < 202311L
#  include <stdbool.h>
#endif

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

enum {
  ARENA_READY      = 0,
  ARENA_NOAVAIL    = -1,
  ARENA_ALLOC_STEP = FIXED_PAGING_SIZE,
};

// Opaque type of struct @Arena.
typedef struct Arena Arena;

// Initialize an empty Arena, can be allocated early if
// should_allocated is true.
bool ArenaInit(Arena **arena, size_t step, bool should_allocate);

// Create a new opaque type @Arena, can be allocated if @should_allocate
// is @true.
Arena *ArenaNew(size_t step, bool should_allocate);

// Return total capacity of the @arena.
size_t ArenaCapacity(Arena *arena);

// Return the offset from left of the @arena.
size_t ArenaOffset(Arena *arena);

// Return an allocated chunk of memory from arena to the
// caller.
void *ArenaAlloc(Arena *arena, size_t size);

// Change the size of the allocated memory.
void *ArenaRealloc(Arena *arena, void *dst, size_t old_size, size_t new_size);

// Freeing the memory hold by arena.
void ArenaDealloc(Arena *arena);

// Return the remaining arena capacity.
size_t ArenaRemaining(Arena *arena);

// Check whether the arena offset is equal to arena capacity
// - 1.
bool ArenaIsFull(Arena *arena);

// Return the inner buffer as new allocated pointer.
void *ArenaPopOut(Arena *arena);

enum AllocMethod {
  FROM_STDLIB,
  FROM_ARENA,
};

/*** SPECIFIC ALLOCATOR ***/
// METHOD:
// 1. @FROM_STDLIB: @malloc, @realloc, @calloc, @posix_memalign
// 2. @FROM_ARENA: @ArenaAlloc, @ArenaRealloc
inline void *SpecialAlloc(enum AllocMethod method, Arena *arena, size_t n,
                          size_t item_size) {
  switch (method) {
  case FROM_ARENA:
    return ArenaAlloc(arena, n * item_size);

  default:
    return calloc(n, item_size);
  }
}

inline void *SpecialRealloc(enum AllocMethod method, Arena *arena, void *dst,
                            size_t old_size, size_t new_size) {
  switch (method) {
  case FROM_ARENA:
    return ArenaRealloc(arena, dst, old_size, new_size);

  default:
    return realloc(dst, new_size);
  }
}

#ifdef __cplusplus
}
}
#endif

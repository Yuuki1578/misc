// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my responsibility at all.
//
// @file arena.h
// @brief linear allocator with segmented region (arena)

// I don't know if bits/size.h exist on all UNIX like system so yeah.
// This device is using __ANDROID_API__ 24.

#pragma once

#ifdef __ANDROID__
#include <bits/page_size.h>
#else
#define PAGE_SIZE 4096
#endif

#include <stdbool.h>
#include <stddef.h>

enum {
    ARENA_READY      = 0,
    ARENA_NOAVAIL    = -1,
    ARENA_ALLOC_STEP = PAGE_SIZE,
};

// Arena data types.
// The arena is responsible for managing its inner buffer.
//
// Task like segmenting pointer, allocating it to another, growing its size
// things like that.
//
// You can initialize the arena once, use it everywhere, and have to free it
// only once.
typedef struct {
    void *rawptr;    // pointer to allocated memory.
    size_t capacity; // total memory that arena can hold.
    size_t offset;   // an offset from the left of the pointer.
    size_t step;     // how much bytes per allocation.
} Arena;

// Create a new Arena, can be allocated early if should_allocated is true.
int arena_init(Arena *arena, size_t step, bool should_allocate);

// Return an allocated chunk of memory from arena to the caller.
void *arena_alloc(Arena *arena, size_t size);

// Change the size of the allocated memory.
void *arena_realloc(Arena *arena, void *dst, size_t old_size, size_t new_size);

// Freeing the memory hold by arena.
void arena_dealloc(Arena *arena);

// Return the remaining arena capacity.
size_t arena_remaining(Arena *arena);

// Return the first memory address from arena.
void *arena_first_addr(Arena *arena);

// Return the last memory address used from arena.
void *arena_last_addr(Arena *arena);

// Return the last memory address from arena.
void *arena_brk_addr(Arena *arena);

// Check whether the arena offset is equal to arena capacity - 1.
bool arena_on_limit(Arena *arena);

// Return the inner buffer as new allocated pointer and freeing the arena.
void *arena_popout(Arena *arena);

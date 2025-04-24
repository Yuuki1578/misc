// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file arena.h
// @brief memory region based allocator (arena)

#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#ifdef _WIN32  // Windows portability
#define PAGE_SIZE 4096
#else
#include <bits/page_size.h>
#endif

#include <stddef.h>

// Default size for arena, work for non-POSIX system
#define ARENA_ALLOC_STEP_INITIALIZER PAGE_SIZE

// Ok
#define ARENA_READY 0

// Fatal
#define ARENA_NOAVAIL -1

// Warn
#define ARENA_BUSY -2

// @brief arena data types
// 
// On 32-bit system, this type is 12 byte in size,
// while on 64-bit system, is 24 byte.
// Each member of this struct can be fetch by
// CPU in each cycle.
// On 64-bit system for example, the CPU may
// fetch 8 byte/cycle, so the CPU need 3 cycle
// for fetching all struct member from memory.
typedef struct Arena Arena;
struct Arena {
  void* rawptr;
  size_t capacity;
  size_t offset;
};

// @brief global arena allocator
// @asociated_function: *_global()
extern Arena ARENA_ALLOCATOR;

// @brief offset for each arena allocation
extern size_t ARENA_STEP_DFL;

extern size_t arena_capacity(Arena* self);
extern size_t arena_remaining(Arena* self);
extern size_t arena_position(Arena* self);
extern void* arena_first_addr(Arena* self);
extern void* arena_last_addr(Arena* self);
extern void* arena_brk_addr(Arena* self);
extern void arena_snapshot(Arena* self);

extern size_t arena_capacity_global(void);
extern size_t arena_remaining_global(void);
extern size_t arena_position_global(void);
extern void* arena_first_addr_global(void);
extern void* arena_last_addr_global(void);
extern void* arena_brk_addr_global(void);
extern void arena_snapshot_global(void);

extern bool arena_reached_limit(Arena* self);
extern int arena_global_initializer(void);
extern int arena_new(Arena* self, size_t dflcap);
extern void* arena_alloc_generic(Arena* self, size_t size);
extern void* arena_alloc(size_t size);
extern void* arena_realloc_generic(Arena* self,
                                   void* dst,
                                   size_t old_size,
                                   size_t new_size);
extern void* arena_realloc(void* dst, size_t old_size, size_t new_size);
extern void arena_dealloc_generic(Arena* self);
extern void arena_dealloc(void);

#endif

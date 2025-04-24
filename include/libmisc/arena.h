#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#include <bits/page_size.h>
#include <stddef.h>

#define ARENA_ALLOC_STEP_INITIALIZER (PAGE_SIZE)
#define ARENA_READY 0
#define ARENA_NOAVAIL -1
#define ARENA_BUSY -2

typedef struct Arena Arena;
struct Arena {
  void* rawptr;
  size_t capacity;
  size_t offset;
};

extern Arena ARENA_ALLOCATOR;
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

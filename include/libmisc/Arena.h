#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#include <stddef.h>

#define ARENA_ALLOC_STEP_INITIALIZER (1 << 10)
#define ARENA_READY 0
#define ARENA_NOAVAIL -1
#define ARENA_BUSY -2

typedef struct Arena Arena;
struct Arena {
  void *rawMemory;
  size_t capacity;
  size_t position;
};

extern Arena ArenaAllocator;
extern size_t ArenaAllocStep;

extern size_t ArenaGetGlobalCapacity(void);
extern size_t ArenaGetGlobalPosition(void);
extern void* ArenaGetGlobalFirstAddress(void);
extern void* ArenaGetGlobalLastAddress(void);
extern void* ArenaGetGlobalBreakAddress(void);

extern bool ArenaReachedLimit(Arena* arenaContext);
extern int ArenaGlobalInitialize(void);
extern int ArenaNew(Arena *arenaContext, size_t defaultCapacity);
extern int ArenaGrow(Arena *arenaContext, size_t size);
extern int ArenaIncrement(Arena *arenaContext, size_t offset);
extern void *ArenaGenericAlloc(Arena *arenaContext, size_t size);
extern void *ArenaAlloc(size_t size);
extern void *ArenaGenericRealloc(Arena *arenaContext, void *dst, size_t size);
extern void *ArenaRealloc(void *dst, size_t size);
extern void ArenaGenericDealloc(Arena *arenaContext);
extern void ArenaDealloc(void);

#endif

#include <libmisc/Arena.h>
#include <stddef.h>
#include <stdlib.h>

Arena ArenaAllocator = {
    .rawMemory = nullptr,
    .capacity = 0,
    .position = 0,
};

size_t ArenaAllocStep = ARENA_ALLOC_STEP_INITIALIZER;

size_t ArenaGetGlobalCapacity(void) {
  return ArenaAllocator.capacity;
}

size_t ArenaGetGlobalPosition(void) {
  return ArenaAllocator.position;
}

void* ArenaGetGlobalFirstAddress(void) {
  return ArenaAllocator.rawMemory - ArenaAllocator.position;
}

void* ArenaGetGlobalLastAddress(void) {
  return ArenaAllocator.rawMemory + ArenaAllocator.position - 1;
}

void* ArenaGetGlobalBreakAddress(void) {
  return ArenaAllocator.rawMemory + ArenaAllocator.capacity - 1;
}

int ArenaGlobalInitialize(void) {
  ArenaAllocator.rawMemory = malloc(ArenaAllocStep);

  if (ArenaAllocator.rawMemory == nullptr) {
    return ARENA_NOAVAIL;
  }

  ArenaAllocator.capacity = ArenaAllocStep;
  return ARENA_READY;
}

int ArenaNew(Arena* arenaContext, size_t defaultCapacity) {
  if (arenaContext == nullptr) {
    return ARENA_NOAVAIL;
  }

  if (defaultCapacity > 0) {
    arenaContext->rawMemory = malloc(defaultCapacity);

    if (arenaContext->rawMemory == nullptr) {
      return ARENA_NOAVAIL;
    }
  }

  arenaContext->capacity = defaultCapacity;
  arenaContext->position = 0;
  return ARENA_READY;
}

int ArenaGrow(Arena* arenaContext, size_t size) {
  if (arenaContext == nullptr || size == 0) {
    return ARENA_NOAVAIL;
  }

  if (arenaContext->capacity == 0) {
    return ArenaNew(arenaContext, size);
  }

  void* temporary = realloc(arenaContext->rawMemory, size);

  if (temporary == nullptr) {
    return ARENA_BUSY;
  }

  arenaContext->rawMemory = temporary;
  arenaContext->capacity += size;
  return ARENA_READY;
}

int ArenaIncrement(Arena* arenaContext, size_t offset) {
  if (arenaContext == nullptr || offset == 0) {
    return ARENA_BUSY;
  }

  if (arenaContext->capacity - 1 == arenaContext->position) {
    int status = ArenaGrow(arenaContext, ArenaAllocStep);

    if (status != ARENA_READY) {
      return status;
    }
  }

  arenaContext->position += offset;
  return ARENA_READY;
}

void* ArenaGenericAlloc(Arena* arenaContext, size_t size) {
  if (arenaContext == nullptr || size == 0) {
    return nullptr;
  }

  if (arenaContext->capacity - 1 == arenaContext->position) {
    int status = ArenaGrow(arenaContext, ArenaAllocStep);

    if (status != ARENA_READY) {
      return nullptr;
    }
  }

  void* ready = arenaContext->rawMemory + arenaContext->position;
  int status = ArenaIncrement(arenaContext, size);

  if (status != ARENA_READY) {
    return nullptr;
  }

  return ready;
}

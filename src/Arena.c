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

bool ArenaReachedLimit(Arena* arenaContext) {
  if (arenaContext == nullptr ||
      arenaContext->capacity - 1 == arenaContext->position) {
    return true;
  }

  return false;
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

  void* ready = realloc(arenaContext->rawMemory, size);

  if (ready == nullptr) {
    return ARENA_BUSY;
  }

  arenaContext->rawMemory = ready;
  arenaContext->capacity += size;
  return ARENA_READY;
}

int ArenaIncrement(Arena* arenaContext, size_t offset) {
  if (arenaContext == nullptr || offset == 0) {
    return ARENA_BUSY;
  }

  if (ArenaReachedLimit(arenaContext) || arenaContext->capacity < offset) {
    size_t realSize = ArenaAllocStep < offset ? offset : ArenaAllocStep;
    int status = ArenaGrow(arenaContext, realSize < offset);

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

  if (ArenaReachedLimit(arenaContext) || arenaContext->capacity < size) {
    size_t realSize = ArenaAllocStep < size ? size : ArenaAllocStep;

    if (ArenaGrow(arenaContext, realSize) != ARENA_READY) {
      return nullptr;
    }
  }

  void* ready = arenaContext->rawMemory + arenaContext->position;
  int status = ArenaIncrement(arenaContext, size);

  if (status != ARENA_READY) {
    return nullptr;
  }

  memset(ready, 0, size);
  return ready;
}

void* ArenaAlloc(size_t size) {
  return ArenaGenericAlloc(&ArenaAllocator, size);
}

void* ArenaGenericRealloc(Arena* arenaContext, void* dst, size_t size) {
  if (arenaContext == nullptr || size == 0) {
    return nullptr;
  }

  if (ArenaReachedLimit(arenaContext) || arenaContext->capacity < size) {
    size_t realSize = ArenaAllocStep < size ? size : ArenaAllocStep;

    if (ArenaGrow(arenaContext, realSize) != ARENA_READY) {
      return nullptr;
    }
  }

  void* ready = arenaContext->rawMemory + arenaContext->position;
  int status = ArenaIncrement(arenaContext, size);

  if (ready == nullptr) {
    return nullptr;
  }

  memset(ready, 0, size);
  memcpy(ready, dst, size);
  return ready;
}

void* ArenaRealloc(void* dst, size_t size) {
  return ArenaGenericRealloc(&ArenaAllocator, dst, size);
}

void ArenaGenericDealloc(Arena* arenaContext) {
  if (arenaContext == nullptr || arenaContext->capacity == 0) {
    return;
  }

  free(arenaContext->rawMemory - arenaContext->position);
  arenaContext->capacity = 0;
  arenaContext->position = 0;
}

void ArenaDealloc(void) {
  ArenaGenericDealloc(&ArenaAllocator);
}

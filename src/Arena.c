#include <libmisc/Arena.h>
#include <stddef.h>
#include <stdlib.h>

void duplicateMemory(void* dst, void* src, size_t size) {
  char* dstByte = dst;
  char* srcByte = src;

  while (size--) {
    *(dstByte++) = *(srcByte++);
  }
}

Arena ArenaAllocator = {
    .rawMemory = nullptr,
    .capacity = 0,
    .position = 0,
};

size_t ArenaAllocStep = ARENA_ALLOC_STEP_INITIALIZER;

size_t ArenaGetCapacity(Arena* arenaContext) {
  return arenaContext != nullptr ? arenaContext->capacity : 0;
}

size_t ArenaGetRemaining(Arena* arenaContext) {
  return arenaContext != nullptr
             ? arenaContext->capacity - arenaContext->position
             : 0;
}

size_t ArenaGetPosition(Arena* arenaContext) {
  return arenaContext != nullptr ? arenaContext->position : 0;
}

void* ArenaGetFirstAddress(Arena* arenaContext) {
  return arenaContext != nullptr ? arenaContext : nullptr;
}

void* ArenaGetLastAddress(Arena* arenaContext) {
  return arenaContext != nullptr
             ? arenaContext->rawMemory + arenaContext->position
             : nullptr;
}

void* ArenaGetBreakAddress(Arena* arenaContext) {
  return arenaContext != nullptr
             ? arenaContext->rawMemory + arenaContext->capacity
             : nullptr;
}

size_t ArenaGetGlobalCapacity(void) {
  return ArenaGetCapacity(&ArenaAllocator);
}

size_t ArenaGetGlobalRemaining(void) {
  return ArenaGetRemaining(&ArenaAllocator);
}

size_t ArenaGetGlobalPosition(void) {
  return ArenaGetPosition(&ArenaAllocator);
}

void* ArenaGetGlobalFirstAddress(void) {
  return ArenaGetFirstAddress(&ArenaAllocator);
}

void* ArenaGetGlobalLastAddress(void) {
  return ArenaGetLastAddress(&ArenaAllocator);
}

void* ArenaGetGlobalBreakAddress(void) {
  return ArenaGetBreakAddress(&ArenaAllocator);
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

  if (ArenaGetCapacity(arenaContext) == 0) {
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

  if (ArenaReachedLimit(arenaContext) || ArenaAllocStep < offset) {
    size_t realSize = ArenaAllocStep;

    while (realSize < offset) {
      realSize *= 2;
    }

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

  if (ArenaReachedLimit(arenaContext) || ArenaAllocStep < size) {
    size_t realSize = ArenaAllocStep;

    while (realSize < size) {
      realSize *= 2;
    }

    if (ArenaGrow(arenaContext, realSize) != ARENA_READY) {
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

void* ArenaAlloc(size_t size) {
  return ArenaGenericAlloc(&ArenaAllocator, size);
}

void* ArenaGenericRealloc(Arena* arenaContext, void* dst, size_t size) {
  if (arenaContext == nullptr || size == 0) {
    return nullptr;
  }

  if (ArenaReachedLimit(arenaContext) || ArenaAllocStep < size) {
    size_t realSize = ArenaAllocStep;

    while (realSize < size) {
      realSize *= 2;
    }

    if (ArenaGrow(arenaContext, realSize) != ARENA_READY) {
      return nullptr;
    }
  }

  // void* ready = arenaContext->rawMemory + arenaContext->position;
  // int status = ArenaIncrement(arenaContext, size);
  void* ready = ArenaGenericAlloc(arenaContext, size);

  if (ready == nullptr) {
    return nullptr;
  }

  // memmove(ready, dst, size);
  duplicateMemory(ready, dst, size);
  return ready;
}

void* ArenaRealloc(void* dst, size_t size) {
  return ArenaGenericRealloc(&ArenaAllocator, dst, size);
}

void ArenaGenericDealloc(Arena* arenaContext) {
  if (arenaContext == nullptr || arenaContext->capacity == 0) {
    return;
  }

  free(arenaContext->rawMemory);
  arenaContext->capacity = 0;
  arenaContext->position = 0;
}

void ArenaDealloc(void) {
  ArenaGenericDealloc(&ArenaAllocator);
}

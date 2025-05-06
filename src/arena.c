// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Shared and static building for this library are provided.
// Any damage caused by this software is not my responsibility at all.

// @file arena.c
// @brief memory region based allocator (arena)

#include <libmisc/arena.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Arena ARENA_ALLOCATOR = {
    .rawptr = nullptr,
    .capacity = 0,
    .offset = 0,
};

size_t ARENA_STEP_DFL = ARENA_ALLOC_STEP_INITIALIZER;

size_t arena_capacity(Arena* self) {
  return self != nullptr ? self->capacity : 0;
}

size_t arena_remaining(Arena* self) {
  return self != nullptr ? self->capacity - self->offset : 0;
}

size_t arena_position(Arena* self) {
  return self != nullptr ? self->offset : 0;
}

void* arena_first_addr(Arena* self) {
  return self != nullptr ? self : nullptr;
}

void* arena_last_addr(Arena* self) {
  return self != nullptr ? self->rawptr + self->offset : nullptr;
}

void* arena_brk_addr(Arena* self) {
  return self != nullptr ? self->rawptr + self->capacity : nullptr;
}

void arena_snapshot(Arena* self) {
  if (self == nullptr) {
    return;
  }

  printf("Arena address:       %p\n", (void*)self);
  printf("Arena first address: %p\n", arena_first_addr(self));
  printf("Arena last address:  %p\n", arena_last_addr(self));
  printf("Arena break address: %p\n", arena_brk_addr(self));
  printf("Arena capacity:      %zu byte\n", arena_capacity(self));
  printf("Arena remaining:     %zu byte\n", arena_remaining(self));
  printf("Arena position:      %zu from left\n", arena_position(self));
  fflush(stdout);
}

size_t arena_capacity_global(void) {
  return arena_capacity(&ARENA_ALLOCATOR);
}

size_t arena_remaining_global(void) {
  return arena_remaining(&ARENA_ALLOCATOR);
}

size_t arena_position_global(void) {
  return arena_position(&ARENA_ALLOCATOR);
}

void* arena_first_addr_global(void) {
  return arena_first_addr(&ARENA_ALLOCATOR);
}

void* arena_last_addr_global(void) {
  return arena_last_addr(&ARENA_ALLOCATOR);
}

void* arena_brk_addr_global(void) {
  return arena_brk_addr(&ARENA_ALLOCATOR);
}

void arena_snapshot_global(void) {
  arena_snapshot(&ARENA_ALLOCATOR);
}

bool arena_reached_limit(Arena* self) {
  if (self == nullptr) {
    return false;
  }

  if (arena_capacity(self) - 1 <= arena_position(self)) {
    return true;
  }

  return false;
}

int arena_global_initializer(void) {
  return arena_new(&ARENA_ALLOCATOR, ARENA_STEP_DFL);
}

int arena_new(Arena* self, size_t dflcap) {
  if (self == nullptr) {
    return ARENA_NOAVAIL;
  }

  if (dflcap == 0) {
    return ARENA_NOAVAIL;
  }

  self->rawptr = malloc(dflcap);

  if (self->rawptr == nullptr) {
    return ARENA_NOAVAIL;
  }

  self->capacity = dflcap;
  self->offset = 0;
  return ARENA_READY;
}

void* arena_alloc_generic(Arena* self, size_t size) {
  void* ready;
  size_t remains;

  if (self == nullptr || size == 0) {
    return nullptr;
  }

  remains = arena_remaining(self);

  if (self->capacity == 0) {
    int status = arena_new(self, ARENA_STEP_DFL);

    if (status != ARENA_READY) {
      return nullptr;
    }
  }

  if (size >= self->capacity || size >= remains) {
    size_t should_above = size > ARENA_STEP_DFL ? size : ARENA_STEP_DFL;
    size_t half_remains = (self->capacity - remains) + should_above;

    // FIXME
    // remains = half_remains + size
    //
    // ptr = 0x0
    // cap = 8
    // offset = 4
    // request = 6
    //
    // cap += request + (cap - offset)
    // request_ptr = ptr + offset
    // offset += request
    //
    // Too large
    // ↓↓↓↓↓↓↓↓↓
    // remains = remains + half_remains + size;
    // 
    // @FIXED April 26, 2025
    // commit hash: cd1419554e83b05954ab290184ff1ed010e4af96
    void* tmp = realloc(self->rawptr, half_remains);

    if (tmp == nullptr) {
      return nullptr;
    }

    self->rawptr = tmp;
    self->capacity += remains;
  }

  // [0, 0, 0, 0, 0, 0]
  //  ^
  // offset
  //
  // [0, 0, 0, 0, 0, 0]
  //              ^
  //            offset
  ready = self->rawptr + self->offset;
  self->offset += size;

  return ready;
}

void* arena_alloc(size_t size) {
  return arena_alloc_generic(&ARENA_ALLOCATOR, size);
}

// This may seem funny, but there is NO WAY i can know
// the exact size of the block of memory before allocation.
void* arena_realloc_generic(
    Arena* self,
    void* dst,
    size_t old_size,  // <- actually life saver, real hero
    size_t new_size) {
  void* ready;

  if (self == nullptr || old_size == 0) {
    return nullptr;
  }

  if ((ready = arena_alloc_generic(self, new_size)) == nullptr) {
    return nullptr;
  }

  if (dst == nullptr) {
    return ready;
  }

  memcpy(ready, dst, old_size);
  return ready;
}

void* arena_realloc(void* dst, size_t old_size, size_t new_size) {
  return arena_realloc_generic(&ARENA_ALLOCATOR, dst, old_size, new_size);
}

void arena_dealloc_generic(Arena* self) {
  if (self == nullptr || self->capacity == 0) {
    return;
  }

  free(self->rawptr);
  self->capacity = 0;
  self->offset = 0;
}

void arena_dealloc(void) {
  arena_dealloc_generic(&ARENA_ALLOCATOR);
}

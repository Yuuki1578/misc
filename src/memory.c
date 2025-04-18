#include <libmisc/memory.h>
#include <stdint.h>
#include <stdlib.h>

Layout layout_new(uint16_t size, size_t default_len) {
  if (size == 0) {
    size = sizeof(char);
  }

  return (Layout){
      .size = size,
      .capacity = 0,
      .requested = default_len,
      .status = LAYOUT_NULL_PTR,
  };
}

void layout_add(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  layout->requested += count;
}

void layout_min(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  if (layout->requested < count) {
    return;
  }

  layout->requested -= count;
}

void *layout_alloc(Layout *layout) {
  void *from_alloc;

  if (layout == nullptr) {
    return malloc(0);
  }

  if (layout->size == 0)
    layout->size = sizeof(char);

  // cannot allocate more than default limit (3GiB)
  if (layout->capacity == 0) {
    from_alloc = malloc(0);

    if (from_alloc == nullptr) {
      layout->status = LAYOUT_NULL_PTR;
    } else {
      layout->status = LAYOUT_UNIQUE_PTR;
    }

    return from_alloc;
  }

  from_alloc = malloc(layout->requested * layout->size);

  if (from_alloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
    layout->capacity += layout->requested * layout->size;
  }

  return from_alloc;
}

void *layout_realloc(Layout *layout, void *dst) {
  size_t current_cap;
  void *from_alloc;

  if (layout == nullptr || dst == nullptr) {
    return layout_alloc(nullptr);
  }

  current_cap = layout->requested * layout->size;
  from_alloc = realloc(dst, current_cap);

  if (from_alloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
    layout->capacity = current_cap;
  }

  return dst;
}

void layout_dealloc(Layout *layout, void *dst) {
  if (layout == nullptr) {
    if (dst != nullptr) {
      free(dst);
    }

    return;
  }

  if (layout->status != LAYOUT_NULL_PTR) {
    free(dst);
  }

  // invalidate the pointer
  dst = nullptr;
  layout->capacity = 0;
  layout->requested = 0;
  layout->status = LAYOUT_NULL_PTR;
}

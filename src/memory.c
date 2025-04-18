#include <ced/memory.h>
#include <stdint.h>
#include <stdlib.h>

Layout layout_new(uint16_t t_size, size_t default_len) {
  if (t_size == 0) {
    t_size = sizeof(char);
  }

  return (Layout){
      .t_size = t_size,
      .cap = t_size * default_len,
      .len = default_len,
      .status = LAYOUT_NULL_PTR,
  };
}

void layout_add(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  layout->cap +=
      ((layout->t_size != 0 ? layout->t_size : sizeof(char)) * count);
  layout->len += count;
}

void layout_min(Layout *layout, size_t count) {
  if (layout == nullptr || count == 0) {
    return;
  }

  if (layout->len < count) {
    return;
  }

  layout->cap -=
      ((layout->t_size != 0 ? layout->t_size : sizeof(char)) * count);
  layout->len -= count;
}

void *layout_alloc(Layout *layout) {
  void *from_alloc;

  if (layout == nullptr) {
    return malloc(0);
  }

  if (layout->t_size == 0)
    layout->t_size = sizeof(char);

  // cannot allocate more than default limit (3GiB)
  if (layout->cap == 0) {
    from_alloc = malloc(0);

    if (from_alloc == nullptr) {
      layout->status = LAYOUT_NULL_PTR;
    } else {
      layout->status = LAYOUT_UNIQUE_PTR;
    }

    return from_alloc;
  }

  from_alloc = malloc(layout->len * layout->t_size);

  if (from_alloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  } else {
    layout->status = LAYOUT_NON_NULL;
  }

  return from_alloc;
}

void *layout_realloc(Layout *layout, void *dst) {
  void *from_alloc;

  if (layout == nullptr || dst == nullptr) {
    return layout_alloc(nullptr);
  }

  from_alloc = realloc(dst, layout->cap);

  if (from_alloc == nullptr) {
    layout->status = LAYOUT_NULL_PTR;
  }

  return dst;
}

void layout_dealloc(Layout *layout, void *dst) {
  if (layout == nullptr) {
    return;
  }

  if (layout->status != LAYOUT_NULL_PTR) {
    free(dst);
  }

  // invalidate the pointer
  dst = nullptr;
  layout->cap = 0;
  layout->len = 0;
  layout->status = LAYOUT_NULL_PTR;
}

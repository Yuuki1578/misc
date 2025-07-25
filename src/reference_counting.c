#include "../include/libmisc/reference_counting.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  mtx_t  mutex;
  void  *raw_data;
  size_t count;
} RefCount;

static inline bool refcount_lock(mtx_t *mutex) {
  switch (mtx_trylock(mutex)) {
  case thrd_error:
    return false;

  case thrd_busy:
    if (mtx_unlock(mutex) != thrd_success)
      return false;

    if (mtx_trylock(mutex) != thrd_success)
      return false;
  }

  return true;
}

static void *get_refcount(const void *object) {
  const uint8_t *counter = object;
  return (void *)(counter - sizeof(RefCount));
}

void *refcount_alloc(const size_t size) {
  RefCount *huge_page;
  uint8_t  *slice;

  if ((huge_page = calloc(sizeof *huge_page + size, 1)) == NULL)
    return NULL;

  if (mtx_init(&huge_page->mutex, mtx_plain) != thrd_success) {
    free(huge_page);
    return NULL;
  }

  huge_page->count    = 1;
  slice               = (void *)huge_page;
  slice               = slice + sizeof *huge_page;
  huge_page->raw_data = slice;

  return (void *)slice;
}

bool refcount_strong(void *object) {
  RefCount *counter;
  if (object == NULL)
    return false;

  counter = get_refcount(object);
  if (refcount_lock(&counter->mutex)) {
    counter->count++;
    mtx_unlock(&counter->mutex);
  } else {
    return false;
  }

  return true;
}

bool refcount_weak(void *object) {
  RefCount *counter;
  bool      mark_as_free;

  if (object == NULL)
    return false;

  counter      = get_refcount(object);
  mark_as_free = false;

  if (refcount_lock(&counter->mutex)) {
    if (counter->count == 0)
      mark_as_free = true;
    else
      counter->count--;

    mtx_unlock(&counter->mutex);
  } else {
    return false;
  }

  if (mark_as_free) {
    mtx_destroy(&counter->mutex);
    free(counter);
  }

  return true;
}

void refcount_drop(void *object) {
  while (refcount_lifetime(object) != 0)
    refcount_weak(object);
}

size_t refcount_lifetime(const void *object) {
  RefCount *counter;
  size_t    object_lifetime;

  if (object == NULL)
    return 0;

  counter = get_refcount(object);
  if (refcount_lock(&counter->mutex)) {
    object_lifetime = counter->count;
    mtx_unlock(&counter->mutex);
  } else {
    return 0;
  }

  return object_lifetime;
}

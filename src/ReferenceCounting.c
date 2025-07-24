#include <libmisc/ReferenceCounting.h>
#include <stdint.h>
#include <stdlib.h>

static inline bool MutexLock(mtx_t *mutex) {
  switch (mtx_trylock(mutex)) {
  case thrd_error:
    return false;

  case thrd_busy:
    if (mtx_unlock(mutex) != thrd_success)
      return false;

    if (mtx_trylock(mutex) != thrd_success)
      return false;

  default:
    return true;
  }
}

static void *GetCounterAddress(void *object) {
  uint8_t *counter = object;
  return (void *)(counter - sizeof(Ref_Count));
}

void *RefCountAlloc(size_t size) {
  Ref_Count *hugePage;
  uint8_t   *slice;

  if ((hugePage = calloc(sizeof *hugePage + size, 1)) == NULL)
    return NULL;

  if (mtx_init(&hugePage->mutex, mtx_plain) != thrd_success) {
    free(hugePage);
    return NULL;
  }

  hugePage->count   = 1;
  slice             = (void *)hugePage;
  slice             = slice + sizeof *hugePage;
  hugePage->rawData = slice;

  return (void *)slice;
}

bool RefCountStrong(void *object) {
  Ref_Count *counter;
  if (object == NULL)
    return false;

  counter = GetCounterAddress(object);
  if (MutexLock(&counter->mutex)) {
    counter->count++;
    mtx_unlock(&counter->mutex);
  } else {
    return false;
  }

  return true;
}

bool RefCountWeak(void *object) {
  Ref_Count *counter;
  bool       markAsFree;

  if (object == NULL)
    return false;

  counter    = GetCounterAddress(object);
  markAsFree = false;

  if (MutexLock(&counter->mutex)) {
    if (counter->count == 0)
      markAsFree = true;
    else
      counter->count--;

    mtx_unlock(&counter->mutex);
  } else {
    return false;
  }

  if (markAsFree) {
    mtx_destroy(&counter->mutex);
    free(counter);
  }

  return true;
}

void RefCountDrop(void *object) {
  while (RefCountLifetime(object) != 0)
    RefCountWeak(object);
}

size_t RefCountLifetime(void *object) {
  Ref_Count *counter;
  size_t     objectLifetime;

  if (object == NULL)
    return 0;

  counter = GetCounterAddress(object);
  if (MutexLock(&counter->mutex)) {
    objectLifetime = counter->count;
    mtx_unlock(&counter->mutex);
  } else {
    return 0;
  }

  return counter->count;
}

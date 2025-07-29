/* The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

#include "../include/libmisc/reference_counting.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    mtx_t mutex;
    void* raw_data;
    size_t count;
} RefCount;

static inline bool refcount_lock(mtx_t* mutex)
{
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

static void* get_refcount(void* object)
{
    const uint8_t* counter = object;
    return (void*)(counter - sizeof(RefCount));
}

void* refcount_alloc(size_t size)
{
    RefCount* huge_page;
    uint8_t* slice;

    if ((huge_page = calloc(sizeof *huge_page + size, 1)) == NULL)
        return NULL;

    if (mtx_init(&huge_page->mutex, mtx_plain) != thrd_success) {
        free(huge_page);
        return NULL;
    }

    huge_page->count = 1;
    slice = (void*)huge_page;
    slice = slice + sizeof *huge_page;
    huge_page->raw_data = slice;

    return (void*)slice;
}

bool refcount_strong(void** object)
{
    RefCount* counter;
    if (object == NULL || *object == NULL)
        return false;

    counter = get_refcount(*object);
    if (refcount_lock(&counter->mutex)) {
        counter->count++;
        mtx_unlock(&counter->mutex);
    } else {
        return false;
    }

    return true;
}

bool refcount_weak(void** object)
{
    RefCount* counter;
    bool mark_as_free;

    if (object == NULL || *object == NULL)
        return false;

    counter = get_refcount(*object);
    mark_as_free = false;

    if (refcount_lock(&counter->mutex)) {
        if (counter->count == 1)
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
        *object = NULL;
    }

    return true;
}

void refcount_drop(void** object)
{
    while (object != NULL && *object != NULL) {
        refcount_lifetime(object);
        refcount_weak(object);
    }
}

size_t refcount_lifetime(void** object)
{
    RefCount* counter;
    size_t object_lifetime;

    if (object == NULL || *object == NULL)
        return 0;

    counter = get_refcount(*object);
    if (refcount_lock(&counter->mutex)) {
        object_lifetime = counter->count;
        mtx_unlock(&counter->mutex);

    } else {
        return 0;
    }

    return object_lifetime;
}

#ifndef MISC_REFCOUNT_H
#define MISC_REFCOUNT_H

#include <stdbool.h>
#include <stddef.h>
#include <threads.h>

/* This is the implementation of a reference counting
originally https://github.com/jeraymond/refcount.git

By default, when you allocate something using malloc() or realloc(), the
object is gone by the time you call free(), but in reference counting, that's
not the case.

You see, if you use reference counting, you can increase it's lifetime count
(Strong), or make it dying (Weak). If the object's lifetime count is zero, the
object is released, however if the object count is still strong, the routine to
make it weak, which is refcount_weak(), is only decrease the object's lifetime
count by 1.

When you make the object strong/weak, there is a guard to make the count only
change in one thread, so that the count doesn't get messed up when you use the
object on a multi-threaded environment, and that guard is called a mutex.

Before you increase/decrease the count, the routine will check if its possible
to obtain a lock of a mutex. If it is, the mutex is locked and the current
thread on which this routine is being called will wait until the mutex is
unlocked. This way you don't encounter data race.

WARNING
Don't ever free() the reference counted object manually, if you want to release
it all, just use refcount_drop(). */

void* refcount_alloc(size_t size);
bool refcount_strong(void** object);
bool refcount_weak(void** object);
void refcount_drop(void** object);
size_t refcount_lifetime(void** object);

#endif

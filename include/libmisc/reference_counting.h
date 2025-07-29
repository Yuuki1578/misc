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

#ifndef MISC_REFERENCE_COUNTING_H
#define MISC_REFERENCE_COUNTING_H

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

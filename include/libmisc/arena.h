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

#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_PAGE (1ULL << 12)

/* Arena types, a single linked list that point to the next allocator.
For a better portability, we using uintptr_t instead of raw void* on a pointer
arithmetic context.

You initialize arena using arena_init() that accept 3 arguments, with the
following:
1. arena, is a pointer to a parent arena that's currently empty.
2. init_size, is the initial size for a memory region that the arena hold.
3. pre_alloc, is a boolean parameter, that let you pre-allocate child_node
   for later use.

The linked list is made of 4 members, described as follow:
1. child_node, is the next allocator that have 2 states, null or nonnull.
2. buffer, is a raw pointer from a libc allocator casted to uintptr_t.
3. size, is the total size of memory region this allocator hold.
4. offset, is the difference between base address and current address.
   This member is always incremented on a call of arena_alloc() or
   arena_realloc()

When you call arena_alloc(), it'll check if the current allocator have memory
as large as "size". If it is, it return the current memory region as a chunk of
memory

The chunk is a result of difference between base address (buffer) and offset,
for example:

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
   ^
 offset

Here we have a 5 worth of bytes with each of respective memory addresses.
If, say, we want 2 bytes chunk from that region, we can do:

  chunk = buffer + offset
  offset += size
  return chunk

Here, we add a difference between buffer and offset, but right now the offset
is zero, so the difference is 5. Now you have a 2 possible outcome:
1. A chunk worth of 2 bytes.
2. A chunk worth more than 2 bytes.

You could use the chunk as if it was 2 bytes of memory, or you could use it as a
chunk of 2 bytes or more, up to 0x4.

Now if you look at our allocator again, things would change

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
             ^
           offset

Ok cool, now we have 3 bytes left, starting at address 0x2.
Now, say, you want more than 2 bytes, what if 5 bytes? can you?

Remember that arena_alloc() will check if the current allocator have enough
chunk for us to take? When the remaining bytes (buffer - offset) is not enough,
arena_alloc() will create a new allocator, pointed by child_node with the
following rules:
1. If the requested size is larger than the size of the past allocator, the size
   of the new allocator would be (size * 2).
2. If it's not, the size will be (past_allocator->size * 2).

The chunk returned by arena_alloc() or arena_realloc() may be NULL, so you must
check it before using it.

Now for the best part is that we only need to free our arena's once and we're
done with it. Other neat thing is that you can use the arena and pass it around
to a bunch of function, so that you know that those section of your program need
to allocate some memory. */

typedef struct Arena {
    struct Arena* next_node;
    size_t size, offset;
    uint8_t* data;
} Arena;

Arena* arena_create(size_t init_size);
void* arena_alloc(Arena** arena, size_t size);
void* arena_realloc(Arena* arena, void* dst, size_t old_size, size_t new_size);
void arena_free(Arena* arena);

#endif

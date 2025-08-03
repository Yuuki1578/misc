#ifndef MISC_ARENA_H
#define MISC_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define ARENA_PAGE (1ULL << 12ULL)
#define remainof(arena) ((arena)->total - (arena)->offset)

/* Arena types, a single linked list that point to the next allocator.
For a better portability, we using uint8_t* instead of raw void* on a pointer
arithmetic context.

You create the arena using arena_create() that accept 1 arguments, the initial size.

The linked list is made of 4 members, described as follow:
1. next, is the next allocator that have 2 states, null or nonnull.
2. data, is a raw pointer from a libc allocator casted to uint8_t*.
3. total, is the total size of memory region this allocator hold.
4. offset, is the difference between base address and current address.
   This member is always incremented on a call of arena_alloc() or
   arena_realloc()

When you call arena_alloc(), it'll check if the current allocator have memory
as large as "size". If it is, it return the current memory region as a chunk of
memory

The chunk is a result of difference between base address (data) and offset,
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
chunk for us to take? When the remaining bytes (total - offset) is not enough,
arena_alloc() will create a new allocator, pointed by @next with the
following rules:
1. If the requested size is larger than the size of the past allocator, the size
   of the new allocator would be (size * 2).
2. If it's not, the size will be (past_allocator->total * 2).

The arena, roughly, would look like this:

[4096] -> [8192] -> [16384] -> [32768]

The chunk returned by arena_alloc() or arena_realloc() may be NULL, so you must
check it before using it.

Now for the best part is that we only need to free our arena's once and we're
done with it. Other neat thing is that you can use the arena and pass it around
to a bunch of function, so that you know that those section of your program need
to allocate some memory.

NOTE:
Since the arena is just a linked list, it search for a suitable arena to perform
allocation by lineary check if the arena is match the requirement (the size).
So it would be better to create a big chunk of arena. The default one is ARENA_PAGE
or 4096 bytes (same as in my system, see the getpagesize(2)), which is enough for
everyone nowadays. */

typedef struct Arena Arena;

struct Arena {
    Arena* next;
    size_t total, offset;
    uint8_t* data;
};

Arena* arena_create(size_t size);
void* arena_alloc(Arena* base, size_t size);
void* arena_realloc(Arena* base, void* dst, size_t old_size, size_t new_size);
void arena_free(Arena* base);

#endif

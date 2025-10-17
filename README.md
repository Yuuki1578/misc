# C23 library for my daily usage

This library provide some basic functionality that C doesn't have.

## Using the header
```c
// For global include path
#include <misc.h>

// Local include path
#include "path/to/misc.h"
```

## Cheatsheet
### Vector
Vector is a container that contain some bytes in order
with a fixed size for each byte. In other words, it's
a generic vector.
```c
#include <misc.h>

#pragma pack(1)
struct Animal {
    char *family;
    int is_alive;
};

int main(void)
{
    Vector pet = vector_create(sizeof(struct Animal));
    struct Animal this_cat = {
        .family = "Feline",
        .is_alive = 1,
    };

    vector_push(&pet, &this_cat);
}
```

### Arena
Arena is just an already allocated region that placed next to each other in a linked list form.
If you allocate from arena, the arena will just chop a chunk from a region and return it to you.
That operation is almost happen in a constant time (if the arena can fulfill the size requirement).

If your system support POSIX API, it's likely that the allocator will use mmap(2) and munmap(2) to
allocate/deallocate the memory. And it's more fast, like so FAST compared to traditional malloc(3)/free(3).
I think it does that because there is no overhead for something like hook function for malloc(3) like
__malloc_hook or option for malloc(3) like mallopt.

On windows, it uses CreateFileMapping() for creating a handle to a mapped memory, MapViewOfFile() and UnmapViewOfFile()
for exposing the memory to the user. If you on POSIX, the struct Arena doesn't contain the HANDLE member, it's windows specific.
```c
#include <misc.h>

int main(void)
{
    Arena* allocator = arena_create(ARENA_PAGE); // 4096
    void *memory = arena_alloc(allocator, 1024); // 4096 - 1024
    memory = arena_alloc(allocator, 1024); // 3072 - 1024
    memory = arena_alloc(allocator, ARENA_PAGE); // Can't fulfill the request, creating a new allocator

    arena_free(allocator); // Free it all at once
}
```

### Manual reference counting
MT-Safe manual reference counting using `RefCount`.
```c
#include <misc.h>

int main(void)
{
    // Create a reference counting object, lifetime now is 1.
    void *pool = refcount_alloc(1 << 12);

    if (Pool != NULL) {
        /* Retain object's lifetime.
        Lifetime now is 2. */
        refcount_upgrade(&pool);

        // Use the object.
        spawn_thread_and_use(&pool);

        /* Release object's lifetime.
        Lifetime now is 1. */
        refcount_degrade(&pool);

        /* Release the object's lifetime until reach 0,
        effectively freeing the object. */
        refcount_drop(&pool);
    }
}
```

### Global Allocator
You can customize the memory allocator used in `Misc.h`.
For example, if you want to use your own allocator, you can define it
on your own:
```c
/*
The allocator API used by Misc.h

#define MISC_ALLOC(Size)
#define MISC_CALLOC(count, size)
#define MISC_REALLOC(ptr, old_size, new_size)
#define MISC_FREE(ptr)


Define those macro and include the header
*/

void *my_alloc(size_t size)
{
    // -- snip --
}

#define MISC_ALLOC(size) my_alloc(size)
#include <misc.h>
```

Or, use the default allocator, the Arena Allocator.
But you can't define the macro for your API
to use in `misc.h` though. If you use the default, then
that's what you got.

```c
#define MISC_USE_GLOBAL_ALLOCATOR
#include <misc.h>

/*
After this, all the allocator will use the Arena allocator API.
The global variable is @_misc_global_allocator
*/

int main(void)
{
    ARENA_INIT(); // Must use!!!

    // Will use the global alloator instead of classic malloc()
    char *buf = read_from_path("Some/File.txt");
    if (buf != NULL)
        printf("%s", buf);

    ARENA_DEINIT(); // Optional, let the OS free the memory from process
}
```

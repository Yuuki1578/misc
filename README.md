# C23 library for my daily usage

This library provide some basic functionality that C doesn't have.

## Using the header
```c
// For global include path
#include <Misc.h>

// Local include path
#include "Path/To/Misc.h"
```

## Cheatsheet
### List
You can create a dynamic list of any type like this:
```c
#include <Misc.h>

int main(void)
{
    List(int) SomeInts = {};
    listAppend(SomeInts, 1);
    listAppend(SomeInts, 2);
    listAppend(SomeInts, 3);

    listFree(SomeInts);
}
```

Or, make an alias to it:
```c
typedef List(char) StringTypes;
```

### Vector
Vector is a container that contain some bytes in order
with a fixed size for each byte. In other words, it's
a generic vector.
```c
#include <Misc.h>

#pragma pack(1)
struct Animal {
    char *Family;
    int IsAlive;
};

int main(void)
{
    Vector MyPet = vectorNew(sizeof(struct Animal));
    struct Animal ThisCat = {
        .Family = "Feline",
        .IsAlive = 1,
    };

    vectorPush(&MyPet, &ThisCat);
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
```c
#include <Misc.h>

int main(void)
{
    Arena* HeadAllocator = arenaCreate(ARENA_PAGE); // 4096
    void *Memory = arenaAlloc(HeadAllocator, 1024); // 4096 - 1024
    Memory = arenaAlloc(HeadAllocator, 1024); // 3072 - 1024
    Memory = arenaAlloc(HeadAllocator, ARENA_PAGE); // Can fulfill the request, new list is appended with a double capacity

    arenaFree(HeadAllocator); // Free it all at once
}
```

### Manual reference counting
MT-Safe manual reference counting using `RefCount`.
```c
#include <Misc.h>

int main(void)
{
    // Create a reference counting object, lifetime now is 1.
    void* Pool = refcountAlloc(1 << 12);

    if (Pool != NULL) {
        /* Retain object's lifetime.
        Lifetime now is 2. */
        refcountUpgrade(&Pool);

        // Use the object.
        spawnThreadAndUse(Pool);

        /* Release object's lifetime.
        Lifetime now is 1. */
        refcountDegrade(&Pool);

        /* Release the object's lifetime until reach 0,
        effectively freeing the object. */
        refcountDrop(&Pool);
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
#define MISC_CALLOC(Count, Size)
#define MISC_REALLOC(Ptr, OldSize, NewSize)
#define MISC_FREE(Ptr)


Define those macro and include the header
*/

void *myAlloc(size_t SizeRequired);
#define MISC_ALLOC(Size) myAlloc(Size)
#include <Misc.h>
```

Or, use the default allocator, the Arena Allocator.
But you can't define the macro for your API
to use in `Misc.h` though. If you use the default, then
that's what you got.

```c
#define MISC_USE_GLOBAL_ALLOCATOR
#include <Misc.h>

/*
After this, all the allocator will use the Arena allocator API.
The global variable is @MiscGlobalAllocator
*/

int main(void)
{
    ARENA_INIT(); // Must use!!!

    // Will use the global alloator instead of classic malloc()
    char *FileContent = fileRead("Some/File.txt");
    if (FileContent != NULL)
        printf("%s", FileContent);

    ARENA_DROP(); // Optional
}
```

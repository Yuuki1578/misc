# C23 library for my daily usage

This library provide some basic functionality that C doesn't have.

## Using the header
```c
// For global include path
#include <misc.h>

// Local include path
#include "path/to/misc.h"
```

## Building the examples
### CMake
```bash
mkdir build && cmake -S . -B build
cmake --build build
```

### Make
```bash
make -j8
```

### Nob
```bash
git clone https://github.com/Yuuki1578/misc.git --recursive
cc nob.c -o nob
./nob
```

## Cheatsheet
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
#include "misc.h"

int main(void)
{
    Arena* allocator = arena_create(ARENA_PAGE); // 4096
    void *memory = arena_alloc(allocator, 1024); // 4096 - 1024
    memory = arena_alloc(allocator, 1024); // 3072 - 1024
    memory = arena_alloc(allocator, ARENA_PAGE); // Can't fulfill the request, creating a new allocator

    arena_free(allocator); // Free it all at once
}
```

### Global Allocator
You can customize the memory allocator used in `misc.h`.
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
#include "misc.h"
```

Or, use the default allocator, the Arena Allocator.
But you can't define the macro for your API
to use in `misc.h` though. If you use the default, then
that's what you got.

```c
#define MISC_USE_GLOBAL_ALLOCATOR
#include "misc.h"

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

### Vector
Vector is one of the containers type in this header. It's generic
using the `void *`. The size of the type is specified in `.item_size` field.
```c
#include "misc.h"

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

    vector_push(&pet, this_cat);
}
```

### String
String is also a containers type. It derives from Vector and can be casted
to each other. So a `String *` is a valid `Vector *`.
```c
#include "misc.h"


int main(void)
{
    String dyn_str = string_fromstr("Hola!", 5);
    string_pushstr(&dyn_str, " worldo\n");
    printf("%s\n", CSTR(dyn_str));

    string_free(&dyn_str);
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

### Single and double linked list
Create a simple queue with `List`:
```c
#include "misc.h"


int main(void)
{
    List list = {.item_size = sizeof(long)};
    long item = 10;
    list_append(&list, &item);

    item = 20;
    list_append(&list, &item);

    item = 30;
    list_append(&list, &item);

    while (list.length != 0) {
        long *item = (long *) list_popleft(&list);
        printf("%li\n", *item);
    }

    list_free(&list);
}
```

### Hash table (unstable)
Finally, this is a containers type that map a sequence of bytes
to an index in a dynamic array using a hash function.
The hash function used here is the **FNV 1a** ***(Fowler/Noll/Vo)***, altough
i'm not even sure if it's the correct implementation.
The collision in this type is handled by chaining the (Key, Value)
pair next onto each other. If the chain is large enough, the table
is expanded. To get the item from the table, the time factor on the
worst case might be `O(64)` on chained key, using the `bsearc()`
would rise up to `O(log n)`. The best case is `O(1)` with the
average of `O(log n)`.

Also note that this feature is only available on C23. It requires
the `typeof()` operator.

```c
#include "misc.h"


int main(void)
{
    HashTable table = hashtable_create();
    unsigned data = 67;

    HashEntry entry_join = {
        .key.key = "Six Seven!!!",
        .key.length = 12,
        ..value = &data,
    };

    if (!hashtable_insert(&table, entry_join))
        return 1;

    HashEntry *entry_retrieve = hashtable_get(&table, entry_join.key);
    if (entry_retrieve != NULL) {
        unsigned *repr = entry_retrieve->value;
        printf("%u\n", *repr);
    }

    hashtable_free(&table);
}
```

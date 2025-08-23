# C23 library for my daily usage

This library provide some basic functionality that C doesn't have.

## Using the header
```c
#include "misc.h"
```

## Cheatsheet
### List
You can create a dynamic list of any type like this:
```c
#include "misc.h"

int main(void)
{
    List(int) some_ints = { 0 };
    list_append(some_ints, 1);
    list_append(some_ints, 2);
    list_append(some_ints, 1);

    list_free(some_ints);
}
```

Or, make an alias to it:
```c
typedef List(char) string_t;
```

### Vector
Vector is a container that contain some bytes in order
with a fixed size for each byte. In other words, it's
a generic vector.
```c
#include "misc.h"

#pragma pack(1)
struct animal {
    char* family;
    int is_alive;
};

int main(void)
{
    Vector my_pet = vector_new(sizeof(struct animal));
    vector_push_many(&my_pet,
        &(struct animal){
            .family = "Feline",
            .is_alive = 0,
        },

        &(struct animal){
            .family = "Caecilia",
            .is_alive = 1,
        }
    );
}
```

### Arena
Arena is just an already allocated region that placed next to each other in a linked list form.
If you allocate from arena, the arena will just chop a chunk from a region and return it to you.
That operation is almost happen in a constant time (if the arena can fulfill the size requirement).
```c
#include "misc.h"

int main(void)
{
    Arena* head = arena_create(ARENA_PAGE); // 4096
    arena_alloc(head, 1024); // 4096 - 1024
    arena_alloc(head, 1024); // 3072 - 1024
    arena_alloc(head, ARENA_PAGE); // Can fulfill the request, new list is appended with a double capacity

    arena_free(head); // Free it all at once
}
```

### Manual reference counting
MT-Safe manual reference counting using `RefCount`.
```c
#include "misc.h"

int main(void)
{
    // Create a reference counting object, lifetime now is 1.
    void* pool = refcount_alloc(1 << 12);

    if (pool != NULL) {
        /* Retain object's lifetime.
        Lifetime now is 2. */
        refcount_strong(&pool);

        // Use the object.
        spawn_thread_and_use(pool);

        /* Release object's lifetime.
        Lifetime now is 1. */
        refcount_weak(&pool);

        /* Release the object's lifetime until reach 0,
        effectively freeing the object. */
        refcount_drop(&pool);
    }
}
```

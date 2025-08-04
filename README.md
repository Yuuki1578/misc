# C23 library for my daily usage

## Building

1. ### Using CMake
```bash
git clone https://github.com/Yuuki1578/misc --recurse-submodule
cd misc

mkdir build
cmake -S . -B build
cmake --build build
```

2. ### Using Makefile
```bash
git clone https://github.com/Yuuki1578/misc --recurse-submodule
cd misc
make -j8
```

3. ### Using nob (recomended)
```bash
git clone https://github.com/Yuuki1578/misc --recurse-submodule
cd misc

clang nob.c -o nob # Or use whatever compiler you are using
./nob
```

## Cheatsheet
1. ### List
You can create a dynamic list of any type like this:
```c
#include "./include/libmisc/list.h"

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

2. ### Vector
Vector is a container that contain some bytes in order
with a fixed size for each byte. In other words, it's
a generic vector.
```c
#include "./include/libmisc/vector.h"

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

3. ### Arena
Arena is just an already allocated region that placed next to each other in a linked list form.
If you allocate from arena, the arena will just chop a chunk from a region and return it to you.
That operation is almost happen in a constant time (if the arena can fulfill the size requirement).
```c
#include "./include/libmisc/arena.h"

int main(void)
{
    Arena* head = arena_create(ARENA_PAGE); // 4096
    arena_alloc(head, 1024); // 4096 - 1024
    arena_alloc(head, 1024); // 3072 - 1024
    arena_alloc(head, ARENA_PAGE); // Can fulfill the request, new list is appended with a double capacity

    arena_free(head); // Free it all at once
}
```

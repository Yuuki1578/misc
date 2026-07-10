# C99 library for my daily usage

This library provide some basic functionality that C doesn't have.

## Using the header
```c
// define this on main file for functions implementation
// main.c
#define MISC_IMPL
#include "misc.h"

// other.c
// Use this in another translation
#include "misc.h"
```

## Building the examples
### Nob
```bash
git clone https://github.com/Yuuki1578/misc.git --recursive
cc nob.c -o nob
./nob
```

## Cheatsheet
### Arena
```c
#include "misc.h"

int main(void)
{
    // See the header file for all available flags
    Arena *allocator = CreateArena(1024, MISC_ARHEAP); // 4096
    void *memory = ArenaAlloc(allocator, 1024); // 1024 - 1024 = 0
    memory = ArenaAlloc(allocator, 1024); // new node
    memory = ArenaAlloc(allocator,1024); // new node

    DestroyArena(allocator); // Free it all at once
}
```

### Dynamic array
```c
#include "misc.h"

typedef struct {
    const char *family;
    int isAlive;
} Animal;

int main(void)
{
    Array(Animal) animals = {0};
    AppendArray(&animals, (Animal){
        .family = "feline",
        .isAlive = true,
    });

    for (uint32_t i = 0; i < animals.len; i++)
        printf("Family: %s, is alive: %d\n",
            animals.items[i].family,
            animals.items[i].isAlive);

    FreeArray(&animals);
}
```

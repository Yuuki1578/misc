# Simple C library for my personal use

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
    arena_t *allocator = arena_init(1024, MISC_ARHEAP); // 4096
    void *memory = arena_alloc(allocator, 1024); // 1024 - 1024 = 0
    memory = arena_alloc(allocator, 1024); // new node
    memory = arena_alloc(allocator,1024); // new node

    arena_free(allocator); // Free it all at once
}
```

### Dynamic array
```c
#include "misc.h"

typedef struct {
    const char *family;
    int is_alive;
} animal_t;

int main(void)
{
    array_t(animal_t) animals = { 0 };
    array_append(&animals, (animal_t){
        .family = "feline",
        .is_alive = true,
    });

    for (uint32_t i = 0; i < animals.len; i++)
        printf("Family: %s, is alive: %d\n",
            animals.items[i].family,
            animals.items[i].is_alive);

    array_free(&animals);
}
```

### Hashmap (C23)
```c
#include "misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    size_t fail = 0;
    for (size_t i = 0; i < 1024 * 50; i++) {
        const size_t K = i << 4;
        bool ok;
        hashmap_insert(&map, K, i, &ok);
        assert(ok);

        size_t *value;
        hashmap_retrieve(&map, K, &value);
        if (value != NULL) {
            printf("[%zu] key: %zu, value: %zu\n", i, K, *value);
        } else {
            fail++;
        }
    }

    printf("retrieve failed count: %zu\n", fail);
    hashmap_free(&map);
    return 0;
```

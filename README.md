# Simple header-only library to get you going with C

This library provide some basic functionalities that C doesn't have,
like generic dynamic-array, generic linked-list, linear allocator,
generic managed-hashmap, string manipulation, and ring buffer.
This will work on every compiler that follows C standard 99.

## Using the header
```c
// define this once in main file for functions implementation
// main.c
#define MISC_IMPL
#include "misc.h"

// Use this in another translation, do not declare MISC_IMPL
// again to avoid linker error.
// other.c
#include "misc.h"
```

As a shared library
```bash
cc -xc misc.h -DMISC_IMPL -shared -fPIC -o libmisc.so
```

## Building the examples
### Nob
```bash
git clone https://github.com/Yuuki1578/misc.git --recursive
cc nob.c -o nob
./nob
```

## Cheatsheet
See the example code in `example/` directory.

## Code style
### 1. All struct must be defined without `typedef`.
```c
struct cpu_info;
```

### 2. All identifiers must uses `snake_case`.
```c
void register_socket(struct socket *socket, struct sockinfo info);
extern i32 global_state_variable;
union maybe_type;
```

### 3. All constants macro/non-macro must uses `SNAKE_CASE_ALL_CAPS`.
```c
#define PI (22.0 / 7.0)
#define MAGIC_NUMBER (0xdeadface)
const f64 global_duration_in_ms = 150.7;
```

### 4. All function-like macros must uses `snake_case`.
```c
#define sec_to_ms(second) ((f64)second * (f64)1000)
#define open_all(path) open(path, O_RDWR)
```

### 5. All functions that is bound to a types must use `typename_function_name()`.
```c
struct socket;
bool socket_connect(struct socket *target, struct socket *dst);
```

### 6. If the types named with 2 or more words, it's appropriate to use abbreviation like `fc_open_file()` or `file_opener_open_file()`.
```c
struct vector_2d;
bool v2d_intersect(struct vector_2d *v2d);
```

### 7. Variables must ne declared in the beginning of the scope
```c
struct hash_map newer = { 0 };
bool ok;

array_try_resize_with(alloc, &newer, into, &ok);
if (!ok)
    return ok;

newer.len = map->len;
for (usize i = 0; i < map->cap; i++) {
    struct hm_entry *entry, *dest;
    entry = &map->items[i];

    if (entry->key == NULL)
        continue;

    dest = hme_find(&newer, entry->key, entry->key_size, entry->hash);
    *dest = *entry;
}

array_free_with(alloc, map);
*map = newer;
return true;
```

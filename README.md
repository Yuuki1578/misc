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

## Building the examples
### Nob
```bash
git clone https://github.com/Yuuki1578/misc.git --recursive
cc nob.c -o nob
./nob
```

## Cheatsheet
See the example code in `example/` directory.

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
See the example code in `example/` directory.

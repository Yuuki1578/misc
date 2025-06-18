# libmisc: dumb C99 library

## Compatibility
Only the `arena.h` and `vector.h` that are platform agnostic,
while others is UNIX only.

## Building
```bash
# Clone repo
git clone https://github.com/Yuuki1578/misc

# Create build directory
cd misc && mkdir build

# Initialize cmake
cmake -S . -B build

# Build
# The library should be a static library or *.a
# It compiles with full optimization flag in mind🥳
cmake --build build
```

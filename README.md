# Dumb C library for my project

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

3. ### Using nob
```bash
git clone https://github.com/Yuuki1578/misc --recurse-submodule
cd misc

clang nob.c -o nob # Or use whatever compiler you are using
./nob
```

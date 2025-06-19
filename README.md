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

## Style Guide / Type Convention
### 1. Type
  - Type name must use `PascalCase` style:
  ```c
  struct ChildProcess {
    int pid;
    int (*on_spawn)(void *args);
  };

  typedef union {
    char byte;
    short half_word;
    int word;
    long double_word;
    __uint64_t quad_word; 
  } SomeWord;
  ```

### 2. Variable and Types Member
  - Variable name must use `snake_case` style, including local, global, and member variable:
  ```c
  struct {
    int foo;
    long bar;
    _Alignas(32) long baz;
  } some_var;
  
  static void *buffer_container = NULL;

  int main(void) {
    auto int some_number = 15;
    auto volatile const char *restrict some_complicated_string = "Hell no";
  }
  ```

### 3. Function
  - Function name must use `PascalCase` style:
  ```c
  static void CreateDirectory(const char *path);
  inline void DoNothing(void) {}
  ```

  - If function is associated with a type, use the `TypeName<Function name>`:
  ```c
  SomeType SomeTypeNew(int foo, char *baz);
  void SomeTypeFree(SomeType *type);
  ```

### 4. File name
  - File name must use `snake_case` followed by file types (*.c*, *.h*):
  ```c
  #include <libmisc/vector.h>
  #include <anyheader/some_comicaly_long_file_name.h>
  ```

### 5. Indentation and Whitespace break
  - Use 2 spaces (not tab) for indentation:
  ```c
  if (true) {
    for (;;) {
      do {
        SomeAction();
      } while (false);
    }
  }
  ```

  - Also for preprocessing directive:
  ```c
  #ifdef _GNU_SOURCE
  #  include <sys/types.h>
  #  define LongOffset off_t
  #  if !defined(_FILE_OFFSET_BITS)
  #    define _FILE_OFFSET_BITS 64
  #  endif
  #endif
  ```

  - Do not break after function name or statement:
  ```c
  // I like this
  if (true) {
    DoSomething(true);
  }

  // chad based C grandpa style
  void SomeFunction(void) {
    // Code here
  }

  // Microsoft'ish
  if (!false)
  {
    GetOut(void);
  }

  // Just don't
  void BadFunction(...)
  {
    // Aw hell no
  }
  ```

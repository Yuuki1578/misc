CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(misc
  DESCRIPTION "Non-standard C23 header only library"
  LANGUAGES C)

SET(CMAKE_C_STANDARD 23)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

INCLUDE_DIRECTORIES(./)

IF (NOT MSVC)
  ADD_COMPILE_OPTIONS(
    -Wall
    -Werror
    -Wextra
    -Wno-overlength-strings
    -Wno-embedded-directive
    -pedantic
    -fomit-frame-pointer
    -funroll-loops
    -ffast-math
    -march=native
    -mtune=native)

  ADD_LINK_OPTIONS(
    -ggdb
    -fsanitize=address)
ENDIF ()

ADD_EXECUTABLE(list examples/list.c)
ADD_EXECUTABLE(file examples/file.c)
ADD_EXECUTABLE(file_reading examples/file_reading.c)
ADD_EXECUTABLE(hexdump examples/hexdump.c)
ADD_EXECUTABLE(forward_list examples/forward_list.c)
ADD_EXECUTABLE(numeric_str examples/numeric_str.c)
ADD_EXECUTABLE(raw_list examples/raw_list.c)
ADD_EXECUTABLE(refcount examples/refcount.c)
ADD_EXECUTABLE(string examples/string.c)
ADD_EXECUTABLE(vector examples/vector.c)
ADD_EXECUTABLE(arena examples/arena.c)
ADD_EXECUTABLE(hash_table examples/hash_table.c)


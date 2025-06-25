cmake_minimum_required(VERSION 3.18)

project(misc
  DESCRIPTION "Miscellanous library for C99"
  HOMEPAGE_URL "https://github.com/Yuuki1578/misc"
  VERSION 0.1.0
  LANGUAGES C
)

set(MISC_SOURCE_DIR "src")
set(MISC_INCLUDE_DIR "include")

add_subdirectory("test")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${MISC_INCLUDE_DIR})

add_compile_options(
  -O3
  -march=native
  -mtune=native
  -ffast-math
  -funroll-loops
  -fomit-frame-pointer
  -Wall
  -Werror
  -Wextra
  -Wno-pointer-arith
  -pedantic
)

add_link_options(
  -s
)

add_library(${PROJECT_NAME}
  STATIC
  ${MISC_SOURCE_DIR}/arena.c
  ${MISC_SOURCE_DIR}/vector.c
  ${MISC_SOURCE_DIR}/tcp.c
  ${MISC_SOURCE_DIR}/mmap.c
  ${MISC_SOURCE_DIR}/file.c
)

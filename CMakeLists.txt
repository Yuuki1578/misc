cmake_minimum_required(VERSION 3.20)
project(misc
  VERSION 0.1.1
  DESCRIPTION "Dumb C89 Library"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

############### LIBMISC ###############
set(MISC_SOURCE_DIR "src")
set(MISC_INCLUDE_DIR "include")

file(GLOB MISC_SOURCE_FILES "${MISC_SOURCE_DIR}/*.c")
add_library(misc STATIC ${MISC_SOURCE_FILES})
target_include_directories(misc PUBLIC ${MISC_INCLUDE_DIR})

target_compile_options(misc PRIVATE
  -Wall
  -Werror
  -Wextra
  -std=c89
  -march=native
  -mtune=native
  -O3
  -g0
  -ffast-math
  -fomit-frame-pointer
  -funroll-loops
)

add_subdirectory(examples)

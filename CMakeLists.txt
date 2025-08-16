cmake_minimum_required(VERSION 3.20)
project(misc
  VERSION 0.1.1
  DESCRIPTION "Dumb C11 Library"
  LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

############### LIBMISC ###############
set(MISC_SOURCE_DIR "src")
set(MISC_INCLUDE_DIR "include")

file(GLOB MISC_SOURCE_FILES "${MISC_SOURCE_DIR}/*.c")
add_library(misc STATIC ${MISC_SOURCE_FILES})
target_include_directories(misc PUBLIC ${MISC_INCLUDE_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(misc PRIVATE
    -Wall
    -Werror
    -Wextra
    -march=native
    -mtune=native
    -O2
    -g0
    -ffast-math
    -fomit-frame-pointer
    -funroll-loops
  )
endif ()

if (NOT MSVC)
  add_subdirectory(examples)
endif ()

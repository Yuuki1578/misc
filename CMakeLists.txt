# The Fuck Around and Find Out License v0.1
# Copyright (C) 2025 Awang Destu Pradhana
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "software"), to deal
# in the software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the software, and to permit persons to whom the software is
# furnished to do so, subject to the following conditions:
#
# 1. The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the software.
#
# 2. The software shall be used for Good, not Evil. The original author of the
# software retains the sole and exclusive right to determine which uses are
# Good and which uses are Evil.
#
# 3. The software is provided "as is", without warranty of any kind, express or
# implied, including but not limited to the warranties of merchantability,
# fitness for a particular purpose and noninfringement. In no event shall the
# authors or copyright holders be liable for any claim, damages or other
# liability, whether in an action of contract, tort or otherwise, arising from,
# out of or in connection with the software or the use or other dealings in the
# software.

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

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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
endif ()

if (NOT MSVC)
  add_subdirectory(examples)
endif ()

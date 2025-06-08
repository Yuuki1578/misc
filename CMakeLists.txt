CMAKE_MINIMUM_REQUIRED(VERSION 3.31)

PROJECT(misc
  DESCRIPTION "Miscellanous library for C"
  HOMEPAGE_URL "https://github.com/Yuuki1578/misc"
  VERSION 0.1.0
  LANGUAGES C
)

SET(MISC_SOURCE_DIR "./src")
SET(MISC_INCLUDE_DIR "./include")

ADD_SUBDIRECTORY("./test")

SET(CMAKE_C_STANDARD 23)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

FILE(GLOB MISC_SOURCE_FILES "${MISC_SOURCE_DIR}/*.c")
INCLUDE_DIRECTORIES(${MISC_INCLUDE_DIR})

ADD_COMPILE_OPTIONS(
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

ADD_LINK_OPTIONS(
  -s
)

ADD_LIBRARY(${PROJECT_NAME}
  STATIC
  ${MISC_SOURCE_FILES}
)

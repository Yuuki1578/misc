CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(misc
  DESCRIPTION "Non-standard C23 header only library"
  LANGUAGES C)

SET(CMAKE_C_STANDARD 23)
SET(CMAKE_C_STANDARD_REQUIRED TRUE)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

INCLUDE_DIRECTORIES(./)

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

ADD_EXECUTABLE(File examples/File.c)
ADD_EXECUTABLE(List examples/List.c)
ADD_EXECUTABLE(Arena examples/Arena.c)
ADD_EXECUTABLE(String examples/String.c)
ADD_EXECUTABLE(Vector examples/Vector.c)
ADD_EXECUTABLE(Hexdump examples/Hexdump.c)
ADD_EXECUTABLE(Numeric examples/Numeric.c)
ADD_EXECUTABLE(RefCount examples/RefCount.c)
ADD_EXECUTABLE(RawDlink examples/RawDlink.c)
ADD_EXECUTABLE(DoubleLink examples/DoubleLink.c)
ADD_EXECUTABLE(LinkedList examples/LinkedList.c)
ADD_EXECUTABLE(FileReading examples/FileReading.c)
ADD_EXECUTABLE(Nob Nob.c)

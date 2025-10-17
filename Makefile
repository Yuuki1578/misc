CC = gcc
CXX = g++
CFLAGS = \
    -Wall \
    -Werror \
    -Wextra \
    -pedantic \
    -ffast-math \
    -fomit-frame-pointer \
    -funroll-loops \
    -march=native \
    -mtune=native \
    -Wno-unused-function \
    -Wno-overlength-strings \
    -std=c17 \
    -O0 \
    -ggdb \
    

SOURCES = \
	examples/list_binding.cpp \
	examples/list.c \
	examples/file.c \
	examples/file_reading.c \
	examples/hexdump.c \
	examples/forward_list.c \
	examples/numeric_str.c \
	examples/raw_list.c \
	examples/refcount.c \
	examples/string.c \
	examples/vector.c \
	examples/arena.c \

$(shell test -d build || mkdir build)
$(shell test -d build/examples || mkdir build/examples)

.PHONY: all clean

all: $(SOURCES:examples/%.c=build/examples/%) $(SOURCES:examples/%.cpp=build/examples/%)
clean:
	@echo Cleaning build artifact
	@rm -rf build

build/examples/%: examples/%.c misc.h
	@echo Compiling $<...
	@$(CC) $< -o $@ $(CFLAGS) -std=c23

build/examples/%: examples/%.cpp misc.h
	@echo Compiling $<...
	@$(CXX) $< -o $@

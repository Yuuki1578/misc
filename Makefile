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

$(shell test -d build/examples || mkdir build/examples -p)

STATIC_LIB = build/libmisc.a

CC = cc
CFLAGS = \
	-Wall \
	-Werror \
	-Wextra \
	-std=c11 \
	-pedantic \
	-ffast-math \
	-fomit-frame-pointer \
	-funroll-loops \
	-O2 \
	-march=native \
	-mtune=native \
	-Iinclude

HOST_OS = $(shell uname -o)

ifeq ($(HOST_OS), Android)
	AR = llvm-ar
else
	AR = ar
endif

ARFLAGS = rcs

C_SOURCES = \
	src/arena.c \
	src/vector.c \
	src/string.c \
	src/reference_counting.c

EXAMPLES = \
	examples/arena.c \
	examples/vector.c \
	examples/string.c \
	examples/reference_counting.c

BUILD_OBJS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
EXAMPLE_OBJS = $(patsubst examples/%.c, build/examples/%, $(EXAMPLES))

.PHONY: all clean

all: $(STATIC_LIB) $(BUILD_OBJS) $(EXAMPLE_OBJS)

build/examples/%: examples/%.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -Wno-overlength-strings -fsanitize=address -Lbuild -lmisc -o $@

$(STATIC_LIB): $(BUILD_OBJS)
	$(AR) $(ARFLAGS) $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf build

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

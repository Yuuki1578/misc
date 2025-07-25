$(shell test -d build/examples || mkdir build/examples -p)

CC = cc
CFLAGS = -Wall -Werror -Wextra -std=c11 -pedantic \
	-ffast-math -fomit-frame-pointer -funroll-loops \
	-O2 -march=native -mtune=native -Iinclude

AR = ar
ARFLAGS = rcs

C_SOURCES = src/Arena.c \
		src/Vector.c \
		src/String.c \
		src/ReferenceCounting.c

EXAMPLES = examples/Arena.c \
		examples/Vector.c \
		examples/String.c \
		examples/ReferenceCounting.c

BUILD_OBJS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
EXAMPLE_OBJS = $(patsubst examples/%.c, build/examples/%.out, $(EXAMPLES))

.PHONY: all examples clean
all: build/libmisc.a $(BUILD_OBJS)
examples: $(EXAMPLE_OBJS)

build/examples/%.out: examples/%.c build/libmisc.a
	$(CC) $(CFLAGS) $< -Wno-overlength-strings -Lbuild -lmisc -o $@

build/libmisc.a: $(BUILD_OBJS)
	$(AR) $(ARFLAGS) $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -rf build

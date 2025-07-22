LIB = libmisc.a
DYLIB = libmisc.so

CC = cc
CFLAGS = -Wall -Werror -Wextra -std=c99 -pedantic \
        -O3 -march=native -mtune=native -funroll-loops \
        -fomit-frame-pointer -ffast-math \

AR = llvm-ar
ARFLAGS = rcs

CFILES = src/vector.c \
		src/arena.c \

EXAMPLES = examples/vector.c

BUILD-DIR = build

ifneq ($(shell [ -d $(BUILD-DIR) ] && echo PRESENT), PRESENT)
    $(shell mkdir $(BUILD-DIR))
endif

ifneq ($(shell [ -d $(BUILD-DIR)/examples ] && echo PRESENT), PRESENT)
    $(shell mkdir $(BUILD-DIR)/examples)
endif

OBJS = $(patsubst src/%.c, $(BUILD-DIR)/%.o, $(CFILES))
EXAMPLES-BIN = $(patsubst examples/%.c, $(BUILD-DIR)/examples/%.bin, $(EXAMPLES))

.PHONY: clean all examples
all: $(BUILD-DIR)/$(LIB) $(BUILD-DIR)/$(DYLIB) $(EXAMPLES-BIN)

$(BUILD-DIR)/$(DYLIB): $(OBJS)
	$(CC) -o $@ $(CFLAGS) -fPIC -shared $^

$(BUILD-DIR)/$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD-DIR)/examples/%.bin: examples/%.c $(BUILD-DIR)/$(LIB)
	$(CC) -o $@ $< -Wl,-rpath $(shell realpath $(BUILD-DIR)) -L$(BUILD-DIR) -lmisc -g -O0 -fsanitize=address

$(BUILD-DIR)/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

clean: $(BUILD-DIR)
	rm -rf $<

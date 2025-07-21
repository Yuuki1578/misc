LIB = libmisc.a

CC = cc
CFLAGS = -Wall -Werror -Wextra -std=c99 -pedantic \
        -O3 -march=native -mtune=native -funroll-loops \
        -fomit-frame-pointer -ffast-math -Iinclude \

AR = ar
ARFLAGS = rcs

CFILES = src/vector.c \
		src/arena.c \

BUILD-DIR = build

ifneq ($(shell [ -d $(BUILD-DIR) ] && echo PRESENT), PRESENT)
    $(shell mkdir $(BUILD-DIR))
endif

OBJS = $(patsubst src/%.c, $(BUILD-DIR)/%.o, $(CFILES))

.PHONY: clean

$(BUILD-DIR)/$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD-DIR)/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

clean: $(BUILD-DIR)
	rm -rf $<

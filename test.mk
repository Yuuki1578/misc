CFLAGS		= -Iinclude -std=c23

test: test.o build/libmisc.a
test.o: test.c

.PHONY: clean

clean:
	rm test test.o

LIBRARY = libmisc.a

CC = cc
CFLAGS = -O3 -Wall -Werror -Wextra \
		 -pedantic -ffast-math \
		 -std=c2y -march=native \
		 -Wno-pointer-arith \
		 -mtune=native -Iinclude \
		 -funroll-loops \
		 -fomit-frame-pointer

AR = ar
ARFLAGS = rcs

SRCDIR = src
SRCS = $(SRCDIR)/arena.c \
	   $(SRCDIR)/nonblock.c

BUILDIR = build
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDIR)/%.o, $(SRCS))

PKGDIR = libmisc

$(BUILDIR)/$(LIBRARY): $(OBJS)
	@echo [INFO]	archiving $^
	@$(AR) $(ARFLAGS) $@ $^

$(BUILDIR)/%.o: $(SRCDIR)/%.c
	@echo [INFO]	compiling $<
	@test -d $(BUILDIR) || mkdir $(BUILDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	@echo [INFO]	cleaning build artifacts
	@rm -rf $(BUILDIR) $(PKGDIR)

pkg: $(PKGDIR)

$(PKGDIR): $(BUILDIR)/$(LIBRARY)
	@echo [INFO]	packaging
	@test -d $@ || mkdir $@
	@mkdir $@/lib
	@cp -r ./include $@
	@cp $^ $@/lib
	@tar -zcf libmisc.tar.gz $@
	@mv libmisc.tar.gz $(BUILDIR)

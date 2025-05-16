LIBRARY		= libmisc.a

CC			= cc
CFLAGS		= -O3 -Wall -Werror -Wextra \
			  -pedantic -ffast-math		\
			  -std=c23 -march=native	\
			  -Wno-pointer-arith		\
			  -mtune=native -Iinclude	\
			  -funroll-loops			\
			  -fomit-frame-pointer

AR			= ar
ARFLAGS		= rcs

SRCDIR		= src
SRCS		= $(SRCDIR)/arena.c \
			  $(SRCDIR)/fs.c

BUILDIR		= build
OBJS		= $(patsubst $(SRCDIR)/%.c, $(BUILDIR)/%.o, $(SRCS))

$(BUILDIR)/$(LIBRARY): $(OBJS)
	@echo [INFO]	archiving $^
	@$(AR) $(ARFLAGS) $@ $^

$(BUILDIR)/%.o: $(SRCDIR)/%.c
	@echo [INFO]	compiling $<
	@test -d $(BUILDIR) || mkdir $(BUILDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean: $(BUILDIR)
	@echo [INFO]	removing $^
	@rm -rf $^

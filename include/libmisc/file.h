#pragma once

#include <libmisc/arena.h>
#include <stdio.h>
#ifdef __unix__
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

void *FileRead(const char *path);
void *FileReadWith(FILE *file);
void *FileReadUsing(const char *path, Arena *arena);

#ifdef __unix__
void *FileReadFrom(int fd);
void *FileReadFromWith(int fd, Arena *arena);
#endif

#ifdef __cplusplus
}
}
#endif

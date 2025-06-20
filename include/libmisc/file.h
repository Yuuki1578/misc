#pragma once

#ifndef _LARGEFILE64_SOURCE
#  define _LARGEFILE64_SOURCE
#endif

#include <fcntl.h>
#include <libmisc/arena.h>
#include <libmisc/types.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#  define MiscOpen(filename, ...)           _open(filename, __VA_ARGS__)
#  define MiscFseek(stream, offset, whench) _fseeki64(stream, offset, whench)
#  define MiscFtell(stream)                 _ftelli64(stream)
#  define MiscSeek(fd, offset, whench)      _lseeki(fd, offset, whench)
#  define MiscFstat(fd, buf)                _fstat64(fd, buf)
typedef struct _stat64 MiscStat64;
typedef i64            MiscOffset64;
#elif defined(__unix__)
#  include <unistd.h>
#  define MiscOpen(filename, ...)           open(filename, __VA_ARGS__)
#  define MiscFseek(stream, offset, whench) fseeko64(stream, offset, whench)
#  define MiscFtell(stream)                 ftello64(stream)
#  define MiscSeek(fd, offset, whench)      lseek64(fd, offset, whench)
#  define MiscFstat(fd, buf)                fstat64(fd, buf)
typedef struct stat64 MiscStat64;
typedef off64_t       MiscOffset64;
#endif

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

void *FILERead(const char *path);
void *FILEReadFromStream(FILE *file);
void *FILEReadUsing(const char *path, Arena *arena);

#ifdef __unix__
void *FdReadFrom(int fd);
void *FdReadUsing(int fd, Arena *arena);
#endif

#ifdef __cplusplus
}
}
#endif

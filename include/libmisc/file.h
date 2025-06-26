#pragma once

#ifndef _LARGEFILE64_SOURCE
#  define _LARGEFILE64_SOURCE
#endif

#include <fcntl.h>
#include <libmisc/arena.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#  ifdef _WIN32
#    define MISC_FILE_OFFSET_BITS 32
#  elif defined(_WIN64)
#    define MISC_FILE_OFFSET_BITS 64
#  endif
#  define MISC_O_RDONLY                     _O_RDONLY
#  define MiscOpen(filename, ...)           _open(filename, __VA_ARGS__)
#  define MiscClose(fd)                     _close(fd)
#  define MiscFileno(stream)                _fileno(stream)
#  define MiscRead(fd, buf, count)          _read(fd, buf, count)
#  define MiscFseek(stream, offset, whench) _fseeki64(stream, offset, whench)
#  define MiscFtell(stream)                 _ftelli64(stream)
#  define MiscSeek(fd, offset, whench)      _lseeki(fd, offset, whench)
#  define MiscFstat(fd, buf)                _fstat64(fd, buf)
typedef struct _stat64 MiscStat;

#elif defined(__unix__) || defined(__linux__)
#  include <unistd.h>
#  ifdef __LP64__
#    define MISC_FILE_OFFSET_BITS 64
#  else
#    define MISC_FILE_OFFSET_BITS 32
#  endif
#  define MISC_O_RDONLY                     O_RDONLY
#  define MiscOpen(filename, ...)           open(filename, __VA_ARGS__)
#  define MiscClose(fd)                     close(fd)
#  define MiscFileno(stream)                fileno(stream)
#  define MiscRead(fd, buf, count)          read(fd, buf, count)
#  define MiscFseek(stream, offset, whench) fseeko64(stream, offset, whench)
#  define MiscFtell(stream)                 ftello64(stream)
#  define MiscSeek(fd, offset, whench)      lseek64(fd, offset, whench)
#  define MiscFstat(fd, buf)                fstat64(fd, buf)
typedef struct stat64 MiscStat;

#else
#  error Your platform doesn't have a POSIX API
#endif

#include <libmisc/types.h>

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

void *FileRead(const char *path, iarch *bytes_readed);
void *FileReadOnly(const char *path);
void *FileReadOnlyWith(const char *path, Arena *arena);
void *FileReadFromFd(int fd, iarch *bytes_readed);
void *FileReadFromFdWith(int fd, iarch *bytes_readed, Arena *arena);
void *FileReadFromStream(FILE *file, iarch *bytes_readed);
void *FileReadFromStreamWith(FILE *file, iarch *bytes_readed, Arena *arena);

#ifdef __cplusplus
}
}
#endif

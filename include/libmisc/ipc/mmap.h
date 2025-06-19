// April 2025, [https://github.com/Yuuki1578/misc.git]
// This is a part of the libmisc library.
// Any damage caused by this software is not my
// responsibility at all.
//
// @file mmap.h
// @brief a pipe-like buffered I/O stream

#pragma once

#if defined(_WIN32) || defined(_WIN64) || !defined(__linux__)
#  error Windows is not supported
#endif

#ifndef __USE_FILE_OFFSET64
#  define __USE_FILE_OFFSET64
#endif

#ifndef __LP64__
#  define __LP64__ 1
#endif

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>

#ifdef __ANDROID__
#  include <bits/seek_constants.h>
#else
#  ifndef SEEK_SET
#    define SEEK_SET 0
#  endif
#  ifndef SEEK_CUR
#    define SEEK_CUR 1
#  endif
#  ifndef SEEK_END
#    define SEEK_END 2
#  endif
#  if defined(__USE_GNU)
#    ifndef SEEK_DATA
#      define SEEK_DATA 3
#    endif
#    ifndef SEEK_HOLE
#      define SEEK_HOLE 4
#    endif
#  endif
#endif

#if __STDC_VERSION__ < 202300L || !defined(__cplusplus)
#  include <stdbool.h>
#endif

// MASK:
enum SetMappingFlags {
  SETMAP_FLAGS  = 0x001,
  SETMAP_PROT   = 0x010,
  SETMAP_OFFSET = 0x011,
  SETMAP_FD     = 0x100,
  SETMAP_SIZE   = 0x101,
  SETMAP_BUFFER = 0x111,
};

// OPAQUE:
typedef struct SetMapping SetMapping;

// Create a new @SetMapping instance with default
// attributes.
SetMapping *SetMappingNew(size_t size);

// Create a new @SetMapping instance with a mapped file.
SetMapping *SetMappingWith(int fd, off_t offset);

// Create a new @SetMapping intance with a copy from @buf up
// to @n bytes
SetMapping *SetMappingFrom(const void *buf, size_t n);

// Configure a @SetMapping instance in @which option.
//
// USAGE:
// 1. SETMAP_FLAGS:  SetMappingConf(map, SETMAP_FLAGS,
// MAP_FD | MAP_SHARED);
// 2. SETMAP_PROT:   SetMappingConf(map, SETMAP_PROT,
// PROT_WRITE | PROT_READ);
// 3. SETMAP_OFFSET: SetMappingConf(map, SETMAP_OFFSET,
// SEEK_CUR, -10);
// 4. SETMAP_FD:     SetMappingConf(map, SETMAP_FD,
// open("./foo", O_RDONLY));
// 5. SETMAP_SIZE:   SetMappingConf(map, SETMAP_SIZE, 1 <<
// 11);
bool SetMappingConf(SetMapping *map, enum SetMappingFlags which, ...);

// Getting a pointer to a @which instance member of
// @SetMapping.
//
// USAGE:
// 1. SETMAP_FLAGS:  SetMappingGet(map, SETMAP_FLAGS);  ->
// int*
// 2. SETMAP_PROT:   SetMappingGet(map, SETMAP_PROT);   ->
// int*
// 3. SETMAP_OFFSET: SetMappingGet(map, SETMAP_OFFSET); ->
// off_t*
// 4. SETMAP_FD:     SetMappingGet(map, SETMAP_FD);     ->
// int*
// 5. SETMAP_SIZE:   SetMappingGet(map, SETMAP_SIZE);   ->
// size_t*
// 6. SETMAP_BUFFER: SetMappingGet(map, SETMAP_BUFFER); ->
// void*
void *SetMappingGet(SetMapping *map, enum SetMappingFlags which);

// Write data to a shared buffer.
ssize_t SetMappingWrite(SetMapping *map, const void *buf, size_t n);

// Read data from a buffer.
ssize_t SetMappingRead(SetMapping *map, void *buf, size_t n);

off_t SetMappingSeek(SetMapping *map, int whench, off_t offset);

// Reset an offset ahead of time.
bool SetMappingRewind(SetMapping *map);

// Close a mapping.
bool SetMappingClose(SetMapping *map);

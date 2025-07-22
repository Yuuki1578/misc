/* The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

#pragma once

#if defined(_WIN32) || defined(_WIN64) || !defined(__linux__)
#  error Windows is not supported
#endif

#ifndef __USE_FILE_OFFSET64
#  define __USE_FILE_OFFSET64
#endif

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

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

#ifdef __LP64__
#  define MISC_FILE_OFFSET_BITS 64
#else
#  define MISC_FILE_OFFSET_BITS 32
#endif

#include <libmisc/types.h>

#if __STDC_VERSION__ < 202300L || !defined(__cplusplus)
#  include <stdbool.h>
#endif

#ifdef __cplusplus
namespace misc {
extern "C" {
#endif

// @SetMappingConf mask.
enum SetMappingFlags {
  SETMAP_FLAGS  = 0x001,
  SETMAP_PROT   = 0x010,
  SETMAP_OFFSET = 0x011,
  SETMAP_FD     = 0x100,
  SETMAP_SIZE   = 0x101,
  SETMAP_BUFFER = 0x111,
};

// Opaque type of struct @SetMapping.
typedef struct SetMapping SetMapping;

// Create a new @SetMapping instance with default
// attributes.
SetMapping *SetMappingNew(size_t size);

// Create a new @SetMapping instance with a mapped file.
SetMapping *SetMappingWith(int fd, Offset offset);

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
// Offset*
// 4. SETMAP_FD:     SetMappingGet(map, SETMAP_FD);     ->
// int*
// 5. SETMAP_SIZE:   SetMappingGet(map, SETMAP_SIZE);   ->
// size_t*
// 6. SETMAP_BUFFER: SetMappingGet(map, SETMAP_BUFFER); ->
// void*
void *SetMappingGet(SetMapping *map, enum SetMappingFlags which);

// Write data to a shared buffer.
ssize_t SetMappingWrite(SetMapping *map, void *buf, size_t n);

// Read data from a buffer.
ssize_t SetMappingRead(SetMapping *map, void *buf, size_t n);

Offset SetMappingSeek(SetMapping *map, int whench, Offset offset);

// Reset an offset ahead of time.
bool SetMappingRewind(SetMapping *map);

// Close a mapping.
bool SetMappingClose(SetMapping *map);

#ifdef __cplusplus
}
}
#endif

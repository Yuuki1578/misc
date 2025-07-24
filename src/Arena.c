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

#include <libmisc/Arena.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

bool ArenaInit(Arena *arena, size_t initSize, bool preAllocate) {
  if (arena == NULL || initSize == 0)
    return false;

  if (preAllocate) {
    if ((arena->nextNode = calloc(1, sizeof *arena)) == NULL)
      return false;
  } else {
    arena->nextNode = NULL;
  }

  if ((arena->buffer = (uintptr_t)calloc(initSize, 1)) == 0) {
    if (preAllocate)
      free(arena->nextNode);

    return false;
  }

  arena->size   = initSize;
  arena->offset = 0;
  return true;
}

static Arena *ArenaPullLast(Arena *base) {
  register Arena *last;
  last = base;

  if (last == NULL)
    return NULL;

  while (true) {
    if (last->nextNode != NULL)
      last = last->nextNode;
    else
      break;
  }

  return last;
}

void *ArenaAlloc(Arena *arena, size_t size) {
  Arena *last;
  void  *result;
  last   = ArenaPullLast(arena);
  result = NULL;

  if (last == NULL || size < 1)
    return NULL;

  if (last->size == 0) {
    if (!ArenaInit(last, size, false))
      return NULL;

  } else if (last->size - last->offset < size) {
    size_t baseValue;
    baseValue = size >= last->size ? size : last->size;

    if ((last = ArenaPullLast(last)) == NULL)
      return NULL;

    if (!ArenaInit(last, baseValue * 2, true))
      return NULL;
  }

  result = (void *)(last->buffer + last->offset);
  last->offset += size;
  return result;
}

void *ArenaRealloc(Arena *arena, void *dst, size_t old_size, size_t new_size) {
  void  *result;
  size_t bytesCopied;
  result      = NULL;
  bytesCopied = 0;

  if (arena == NULL)
    return NULL;

  if (old_size == new_size)
    return dst;

  if (dst == NULL)
    return ArenaAlloc(arena, new_size);

  if ((result = ArenaAlloc(arena, new_size)) == NULL)
    return NULL;

  bytesCopied = old_size > new_size ? new_size : old_size;
  memcpy(result, dst, bytesCopied);
  return result;
}

void ArenaFree(Arena *arena) {
  Arena *current;

  for (current = arena; current != NULL;) {
    if (current->size > 0)
      free((void *)current->buffer);

    if (current->nextNode != NULL)
      current = current->nextNode;
    else
      break;
  }
}

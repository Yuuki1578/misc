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

#include "../include/libmisc/arena.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
  Arena    arena;
  int64_t *big_chunk;

  assert(arena_init(&arena, 4096, true));
  big_chunk = arena_alloc(&arena, 64 * sizeof *big_chunk);
  assert(big_chunk);

  for (int64_t i = 0; i < 64; i++) {
    big_chunk[i] = i * 2;
    printf("Chunk: %li\n", big_chunk[i]);
  }

  big_chunk = arena_realloc(&arena, big_chunk, 64 * sizeof *big_chunk,
                            128 * sizeof *big_chunk);

  assert(big_chunk);
  for (int64_t i = 0; i < 128; i++) {
    printf("Chunk: %li\n", big_chunk[i]);
  }

  arena_free(&arena);
}

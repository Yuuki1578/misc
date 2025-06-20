#include <libmisc/arena.h>
#include <libmisc/file.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum Allocator {
  ALLOC_STDLIB,
  ALLOC_ARENA,
};

static bool GetFileSize(FILE *file, MiscOffset64 *offset) {
  if (MiscFseek(file, 0, SEEK_END) != -1)
    if ((*offset = MiscFtell(file)) != -1)
      return MiscFseek(file, 0, SEEK_SET) != -1 ? true : false;

  return false;
}

static void *FileReadFromStreamCompact(FILE *file, enum Allocator allocator,
                                       Arena *arena) {
  void        *buffer;
  MiscOffset64 seek;

  if (file == NULL)
    return NULL;

  if (ferror(file) != 0)
    clearerr(file);

  if (!GetFileSize(file, &seek))
    return NULL;

  switch (allocator) {
  case ALLOC_STDLIB:
    buffer = calloc(seek, 1);
    break;

  case ALLOC_ARENA:
    buffer = ArenaAlloc(arena, seek);
    break;
  }

  if (buffer == NULL)
    return NULL;

  if (fread(buffer, 1, seek - 1, file) == 0) {
    free(buffer);
    return NULL;
  }

  return buffer;
}

void *FILEReadFromStream(FILE *file) {
  return FileReadFromStreamCompact(file, ALLOC_STDLIB, NULL);
}

static void *FileReadCompact(const char *path, enum Allocator allocator,
                             Arena *arena) {
  FILE *file = fopen(path, "rb");
  void *buffer;

  if (file == NULL)
    return NULL;

  switch (allocator) {
  case ALLOC_STDLIB:
    buffer = FileReadFromStreamCompact(file, allocator, NULL);
    break;

  case ALLOC_ARENA:
    buffer = FileReadFromStreamCompact(file, allocator, arena);
    break;
  }

  fclose(file);
  return buffer != NULL ? buffer : NULL;
}

void *FILERead(const char *path) {
  return FileReadCompact(path, ALLOC_STDLIB, NULL);
}

void *FILEReadUsing(const char *path, Arena *arena) {
  return FileReadCompact(path, ALLOC_ARENA, arena);
}

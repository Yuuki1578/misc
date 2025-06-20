#include <libmisc/arena.h>
#include <libmisc/file.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum Allocator {
  ALLOC_STDLIB,
  ALLOC_ARENA,
};

static void *FileReadCompact(int fd, const char *path, enum Allocator allocator,
                             Arena *arena, iarch *bytes_readed) {
  void    *buffer;
  MiscStat file_stat;
  iarch    readed;

  if (fd == -1)
    if ((fd = MiscOpen(path, MISC_O_RDONLY)) == -1)
      return NULL;

  if (MiscFstat(fd, &file_stat) != 0) {
    if (path != NULL)
      MiscClose(fd);

    return NULL;
  }

  switch (allocator) {
  case ALLOC_STDLIB:
    buffer = calloc(file_stat.st_size + 1, 1);
    break;

  case ALLOC_ARENA:
    buffer = ArenaAlloc(arena, file_stat.st_size + 1);
    break;
  }

  if (buffer == NULL) {
    if (path != NULL)
      MiscClose(fd);

    return NULL;
  }

  if ((readed = MiscRead(fd, buffer, file_stat.st_size)) < 0) {
    if (path != NULL)
      MiscClose(fd);

    if (allocator == ALLOC_STDLIB)
      free(buffer);

    return NULL;
  }

  if (path != NULL)
    MiscClose(fd);

  if (bytes_readed != NULL)
    *bytes_readed = readed;

  return buffer;
}

void *FileRead(const char *path, iarch *bytes_readed) {
  return FileReadCompact(-1, path, ALLOC_STDLIB, NULL, bytes_readed);
}

void *FileReadOnly(const char *path) {
  return FileReadCompact(-1, path, ALLOC_STDLIB, NULL, NULL);
}

void *FileReadOnlyWith(const char *path, Arena *arena) {
  return FileReadCompact(-1, path, ALLOC_ARENA, arena, NULL);
}

void *FileReadFromFd(int fd, iarch *bytes_readed) {
  return FileReadCompact(fd, NULL, ALLOC_STDLIB, NULL, bytes_readed);
}

void *FileReadFromFdWith(int fd, iarch *bytes_readed, Arena *arena) {
  return FileReadCompact(fd, NULL, ALLOC_ARENA, arena, bytes_readed);
}

void *FileReadFromStream(FILE *file, iarch *bytes_readed) {
  return FileReadCompact(fileno(file), NULL, ALLOC_STDLIB, NULL, bytes_readed);
}

void *FileReadFromStreamWith(FILE *file, iarch *byted_readed, Arena *arena) {
  return FileReadCompact(fileno(file), NULL, ALLOC_ARENA, arena, byted_readed);
}

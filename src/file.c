#include <libmisc/arena.h>
#include <libmisc/file.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static void *FileReadCompact(int fd, const char *path, enum AllocMethod method,
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

  buffer = SpecialAlloc(method, arena, file_stat.st_size + 1, 1);
  if (buffer == NULL) {
    if (path != NULL)
      MiscClose(fd);

    return NULL;
  }

  if ((readed = MiscRead(fd, buffer, file_stat.st_size)) < 0) {
    if (path != NULL)
      MiscClose(fd);

    if (method == FROM_STDLIB)
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
  return FileReadCompact(-1, path, FROM_STDLIB, NULL, bytes_readed);
}

void *FileReadOnly(const char *path) {
  return FileReadCompact(-1, path, FROM_STDLIB, NULL, NULL);
}

void *FileReadOnlyWith(const char *path, Arena *arena) {
  return FileReadCompact(-1, path, FROM_ARENA, arena, NULL);
}

void *FileReadFromFd(int fd, iarch *bytes_readed) {
  return FileReadCompact(fd, NULL, FROM_STDLIB, NULL, bytes_readed);
}

void *FileReadFromFdWith(int fd, iarch *bytes_readed, Arena *arena) {
  return FileReadCompact(fd, NULL, FROM_ARENA, arena, bytes_readed);
}

void *FileReadFromStream(FILE *file, iarch *bytes_readed) {
  return FileReadCompact(MiscFileno(file), NULL, FROM_STDLIB, NULL,
                         bytes_readed);
}

void *FileReadFromStreamWith(FILE *file, iarch *byted_readed, Arena *arena) {
  return FileReadCompact(MiscFileno(file), NULL, FROM_ARENA, arena,
                         byted_readed);
}

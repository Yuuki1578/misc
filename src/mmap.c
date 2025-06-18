#include <libmisc/ipc/mmap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SETMAP_DEFAULT                                                         \
  (SetMapping) {                                                               \
    .buffer = MAP_FAILED, .offset = 0, .size = 0,                              \
    .prot = PROT_WRITE | PROT_READ, .flags = MAP_ANON | MAP_SHARED, .fd = -1,  \
  }

#define SETMAP_ADDITIONAL_SIZE (2 << 3)

struct SetMapping {
  void   *buffer;
  off64_t offset;
  size_t  size;
  int     prot;
  int     flags;
  int     fd;
};

bool SetMappingConf(SetMapping *map, enum SetMappingFlags which, ...) {
  va_list va;
  int     whench;
  off64_t offset;
  size_t  new_size, try_end;
  void   *tmp;

  if (map == NULL)
    return false;

  va_start(va, which);

  switch (which) {
  case SETMAP_FLAGS:
    map->flags = va_arg(va, int);
    break;

  case SETMAP_PROT:
    map->prot = va_arg(va, int);
    break;

  case SETMAP_OFFSET:
    whench = va_arg(va, int);
    offset = va_arg(va, off64_t);

    if (map->offset < 0)
      map->offset = 0;

    switch (whench) {
    case SEEK_SET:
      if (offset < 0 || (size_t)offset > map->size - 1) {
        va_end(va);
        return false;
      }

      map->offset = offset;
      break;

    case SEEK_CUR:
      if (map->offset + offset < 0 ||
          (size_t)(map->offset + offset) > map->size - 1) {
        va_end(va);
        return false;
      }

      map->offset += offset;
      break;

    case SEEK_END:
      try_end = (map->size - 1) - offset;
      if (try_end < 0 || (size_t)try_end > map->size - 1) {
        va_end(va);
        return false;
      }

      map->offset = try_end;
      break;
    }
    break;

  case SETMAP_FD:
    map->fd = va_arg(va, int);
    if (map->buffer == NULL || map->buffer == MAP_FAILED) {
      map->buffer =
          mmap(NULL, map->size, map->prot, map->flags, map->fd, map->offset);
      if (map->buffer == MAP_FAILED) {
        va_end(va);
        return false;
      }
    } else {
      if (read(map->fd, map->buffer + map->offset, map->size - map->offset) <
          0) {
        va_end(va);
        return false;
      }
    }

    break;

  case SETMAP_SIZE:
    new_size = va_arg(va, size_t);
    if (map->buffer == NULL || map->buffer == MAP_FAILED) {
      tmp = mmap(NULL, new_size, map->prot, map->flags, map->fd, map->offset);
    } else {
      tmp = mremap(map->buffer, map->size, new_size, MREMAP_MAYMOVE);
    }

    if (tmp == MAP_FAILED) {
      va_end(va);
      return false;
    }

    map->buffer = tmp;
    map->size   = new_size;
    break;

  default:
    return false;
  }

  va_end(va);
  return true;
}

SetMapping *SetMappingNew(size_t size) {
  SetMapping *map = mmap(NULL, sizeof(SetMapping), PROT_WRITE | PROT_READ,
                         MAP_ANON | MAP_SHARED, -1, 0);

  if (map == MAP_FAILED)
    return NULL;

  *map        = SETMAP_DEFAULT;
  map->size   = size;
  map->buffer = mmap(NULL, size, map->prot, map->flags, map->fd, map->offset);

  if (map->buffer == MAP_FAILED) {
    munmap(map, sizeof(SetMapping));
    return NULL;
  }

  return map;
}

SetMapping *SetMappingWith(int fd, off64_t offset) {
  SetMapping *map;
  struct stat file_stat = {0};

  if (fstat(fd, &file_stat) != 0)
    return NULL;

  map = SetMappingNew(file_stat.st_size);
  if (map == NULL)
    return NULL;

  if (!SetMappingConf(map, SETMAP_FD, fd)) {
    SetMappingClose(map);
    return NULL;
  }

  if (!SetMappingConf(map, SETMAP_OFFSET, SEEK_SET, offset)) {
    SetMappingClose(map);
    return NULL;
  }

  return map;
}

SetMapping *SetMappingFrom(const void *buf, size_t n) {
  SetMapping *map = SetMappingNew(n);
  void       *inner;

  if (map == NULL)
    return NULL;

  inner = SetMappingGet(map, SETMAP_BUFFER);
  if (inner == NULL) {
    SetMappingClose(map);
    return NULL;
  }

  memcpy(inner, buf, n);
  return map;
}

void *SetMappingGet(SetMapping *map, enum SetMappingFlags which) {
  if (map == NULL)
    return NULL;

  switch (which) {
  case SETMAP_BUFFER:
    return map->buffer;

  case SETMAP_OFFSET:
    return &map->offset;

  case SETMAP_SIZE:
    return &map->size;

  case SETMAP_PROT:
    return &map->prot;

  case SETMAP_FLAGS:
    return &map->flags;

  case SETMAP_FD:
    return &map->fd;

  default:
    return NULL;
  }
}

ssize_t SetMappingWrite(SetMapping *map, const void *buf, size_t n) {
  size_t remains;

  if (map == NULL)
    return -1;

  if (buf == NULL)
    return -1;

  if ((map->size - 1) - map->offset <= 0)
    return 0;

  if (map->offset < 0)
    map->offset = 0;

  if (n > (remains = (map->size - 1) - map->offset))
    n = remains;

  if (map->buffer == MAP_FAILED)
    if (!SetMappingConf(map, SETMAP_SIZE, n))
      return -1;

  memcpy(map->buffer + map->offset, buf, n);
  map->offset += n;
  return n;
}

ssize_t SetMappingRead(SetMapping *map, void *buf, size_t n) {
  size_t remains;

  if (map == NULL || map->buffer == MAP_FAILED)
    return -1;

  if (buf == NULL)
    return -1;

  if ((map->size - 1) - map->offset <= 0)
    return 0;

  if (map->offset < 0)
    map->offset = 0;

  if (n > (remains = (map->size - 1) - map->offset))
    n = remains;

  memcpy(buf, map->buffer + map->offset, n);
  map->offset += n;
  return n;
}

off64_t SetMappingSeek(SetMapping *map, int whench, off64_t offset) {
  if (!SetMappingConf(map, SETMAP_OFFSET, whench, offset))
    return -1;

  return *(off64_t *)SetMappingGet(map, SETMAP_OFFSET);
}

bool SetMappingRewind(SetMapping *map) {
  return SetMappingConf(map, SETMAP_OFFSET, SEEK_SET, 0);
}

bool SetMappingClose(SetMapping *map) {
  if (map == NULL || map == MAP_FAILED)
    return false;

  if (map->buffer != NULL || map->buffer != MAP_FAILED)
    munmap(map->buffer, map->size);

  if (map->fd != -1)
    close(map->fd);

  return munmap(map, sizeof(SetMapping)) == 0 ? true : false;
}

/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under the MIT License. All rights reserved.

######          ######   ###     #############         ###############
######          ######   ###   ################     #####################
### ###        ### ###   ###   ###           ###   ###                ###
###  ###      ###  ###   ###   ###                 ###
###   ###    ###   ###   ###   ###                 ###
###    ###  ###    ###   ###     #############     ###
###     ######     ###   ###                 ###   ###
###                ###   ###                 ###   ###
###                ###   ###                 ###   ###
###                ###   ###   ###           ###   ###                ###
###                ###   ###   #################     ####################
###                ###   ###     #############         ###############


*/

#ifndef MISC_H
#define MISC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef uintptr_t usize;
typedef intptr_t isize;
typedef float f32;
typedef double f64;
typedef long double f80;

#define MISC_ALIGN (sizeof(void *))

#define isValidAlloc(allocptr)                                                 \
  ((allocptr)->alloc != NULL && (allocptr)->realloc != NULL &&                 \
   (allocptr)->free != NULL)

#define isValidAlign(alignment)                                                \
  ((alignment) != 0 && ((alignment) & ((alignment) - 1)) == 0)

#define alignPtr(ptr, align)                                                   \
  ((void *)(((uintptr_t)(ptr) + (align) - 1) & ~((align) - 1)))

#define alignSizeUp(size) ((uintptr_t)alignPtr(size, 8))

#define alignedAlloc(size, alignment)                                          \
  calloc((usize)alignPtr(size, alignment), 1)

#define alignedRealloc(ptr, size, alignment)                                   \
  realloc(ptr, (uintptr_t)alignPtr(size, alignment))

#define panicFormat(msg)                                                       \
  do {                                                                         \
    fprintf(stderr, "FILE: %s, LINE: %d, cause: \"%s\"\n", __FILE__, __LINE__, \
            (msg));                                                            \
    abort();                                                                   \
  } while (0)

#define runtimeAssert(cond, msg)                                               \
  do {                                                                         \
    if (!(cond))                                                               \
      panicFormat(msg);                                                        \
  } while (0)

typedef struct LinkedList {
  struct LinkedList *next;
  // ...
} LinkedList;

LinkedList *linkedListNew(usize size);
LinkedList *linkedListPutAfter(LinkedList *node, usize size);
LinkedList *linkedListPutBefore(LinkedList *node, usize size);
void linkedListFree(LinkedList *node);
LinkedList *linkedListPopAfter(LinkedList *node);
LinkedList *linkedListLast(LinkedList *node);
usize linkedListLength(LinkedList *node);
void *linkedListValue(LinkedList *node);

#ifdef MISC_IMPL

LinkedList *linkedListNew(usize size) {
  LinkedList *node = alignedAlloc(size, 1);
  if (node != NULL)
    node->next = NULL;

  return node;
}

LinkedList *linkedListPutAfter(LinkedList *node, usize size) {
  LinkedList *next = linkedListNew(size);
  node->next = next;
  return next;
}

LinkedList *linkedListPutBefore(LinkedList *node, usize size) {
  LinkedList *before = linkedListNew(size);
  if (before != NULL)
    before->next = node;

  return before;
}

void linkedListFree(LinkedList *node) {
  while (node != NULL) {
    LinkedList *tmp = node->next;
    free(node);
    node = tmp;
  }
}

LinkedList *linkedListPopAfter(LinkedList *node) {
  LinkedList *next = node->next;
  LinkedList *tmp = next != NULL ? next->next : NULL;
  node->next = tmp;
  return next;
}

LinkedList *linkedListLast(LinkedList *node) {
  if (node == NULL)
    return NULL;

  while (node->next != NULL)
    node = node->next;

  return node;
}

void *linkedListValue(LinkedList *node) { return (u8 *)node + sizeof *node; }

usize linkedListLength(LinkedList *node) {
  usize count = 0;
  while (node != NULL)
    node = node->next, count++;

  return count;
}

#endif

typedef struct Arena Arena;

Arena *arenaNew(usize size);
void *arenaAlloc(Arena *arena, usize size);
void *arenaRealloc(Arena *arena, void *ptr, usize oldSize, usize newSize);
void arenaFree(Arena *arena);
bool arenaAlign(Arena *arena, usize alignment);
usize arenaSize(Arena *arena);

#ifdef MISC_IMPL
typedef struct {
  usize cap;
  usize len;
  // ...
} ArenaBody;

struct Arena {
  LinkedList *head, *last;
};

Arena *arenaNew(usize size) {
  Arena *arena;
  ArenaBody body = {0}, *value;

  if (size < 1)
    return NULL;

  arena = alignedAlloc(sizeof *arena, 1);
  if (arena == NULL)
    return NULL;

  body.cap = size;
  arena->head = linkedListNew(sizeof body + size);
  if (arena->head == NULL) {
    free(arena);
    return NULL;
  }

  arena->last = arena->head;
  value = linkedListValue(arena->head);
  *value = body;
  return arena;
}

void *arenaAlloc(Arena *arena, usize size) {
  LinkedList *last;
  ArenaBody *body;
  void *ptr;

  if (arena == NULL || size < 1)
    return NULL;

  last = arena->last;
  body = linkedListValue(last);

  if (body->cap - body->len < size) {
    usize new_size = (body->cap > size ? body->cap : size) + size;
    ArenaBody newer = {.cap = new_size};
    LinkedList *newTail = linkedListPutAfter(last, sizeof newer + new_size);

    if (newTail == NULL)
      return NULL;

    arena->last = newTail;
    last = arena->last;
    body = linkedListValue(last);
    *body = newer;
  }

  ptr = (u8 *)body + sizeof *body + body->len;
  body->len += size;
  return ptr;
}

void *arenaRealloc(Arena *arena, void *ptr, usize oldSize, usize newSize) {
  void *newer;
  usize trueSize;

  if (arena == NULL || newSize == 0)
    return NULL;

  newer = arenaAlloc(arena, newSize);
  if (ptr == NULL)
    return newer;

  trueSize = oldSize > newSize ? newSize : oldSize;
  return memmove(newer, ptr, trueSize);
}

void arenaFree(Arena *arena) {
  if (arena != NULL) {
    linkedListFree(arena->head);
    free(arena);
  }
}

usize arenaSize(Arena *arena) {
  LinkedList *node;
  usize size = 0;

  if (arena == NULL)
    return size;

  node = arena->head;
  while (node != NULL) {
    ArenaBody *body = linkedListValue(node);
    size += body->cap;
    node = node->next;
  }
  return size;
}

bool arenaAlign(Arena *arena, usize alignment) {
  ArenaBody *body, newer;
  LinkedList *next;
  usize aligned;

  body = linkedListValue(arena->last);
  aligned = (usize)alignPtr(body->len, alignment);

  if (aligned >= body->cap) {
    newer = (ArenaBody){.cap = aligned};
    if ((next = linkedListPutAfter(arena->last, sizeof newer)) != NULL) {
      *(ArenaBody *)linkedListValue(next) = newer;
      arena->last = next;
    } else {
      return false;
    }
  } else {
    body->len = aligned;
  }

  return true;
}

#endif

#ifndef MISC_ARRAY_RESERVE
#define MISC_ARRAY_RESERVE (8)
#endif

#define arrayIsEmpty(array)                                                    \
  ((array) != NULL ? ((array)->items == NULL || (array)->cap < 1) : 1)

#define arrayRemains(array)                                                    \
  ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define arrayTryResize(array, N, ok)                                           \
  do {                                                                         \
    if ((N) <= 0) {                                                            \
      free((array)->items);                                                    \
      memset((array), 0, sizeof *(array));                                     \
      *(ok) = 1;                                                               \
    } else {                                                                   \
      void *tmp;                                                               \
      if ((array)->cap == 0) {                                                 \
        tmp = alignedAlloc((N) * sizeof *(array)->items, 1);                   \
      } else {                                                                 \
        tmp = alignedRealloc((array)->items, (N) * sizeof *(array)->items, 1); \
      }                                                                        \
      if (tmp != NULL) {                                                       \
        *(ok) = 1;                                                             \
        (array)->items = tmp;                                                  \
        (array)->cap = (N);                                                    \
        if ((N) < (array)->len) {                                              \
          (array)->len = (N);                                                  \
        }                                                                      \
      } else {                                                                 \
        *(ok) = 0;                                                             \
      }                                                                        \
    }                                                                          \
  } while (0)

#define arrayResize(array, N)                                                  \
  do {                                                                         \
    bool ok = false;                                                           \
    arrayTryResize(array, N, &ok);                                             \
    runtimeAssert(ok, "arrayTryResize() failed");                              \
  } while (0)

#define arrayTryAppend(array, item, ok)                                        \
  do {                                                                         \
    if ((array)->cap <= (array)->len) {                                        \
      arrayTryResize(array, (array)->cap + MISC_ARRAY_RESERVE, ok);            \
    }                                                                          \
    if (*(ok)) {                                                               \
      (array)->items[(array)->len++] = (item);                                 \
    }                                                                          \
  } while (0)

#define arrayAppend(allocator, array, item)                                    \
  do {                                                                         \
    bool ok = false;                                                           \
    arrayTryAppend(array, item, &ok);                                          \
    runtimeAssert(ok, "arrayTryAppend() failed");                              \
  } while (0)

#define arrayTryExtend(array, n_items, N, ok)                                  \
  do {                                                                         \
    if ((n_items) != NULL && (N) > 0) {                                        \
      if (arrayIsEmpty(array) || arrayRemains(array) <= (N)) {                 \
        arrayTryResize(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok);    \
        if (!*(ok)) {                                                          \
          break;                                                               \
        }                                                                      \
      }                                                                        \
      memmove((array)->items + (array)->len, (n_items),                        \
              (N) * sizeof *(array)->items);                                   \
      (array)->len += (N);                                                     \
      *(ok) = 1;                                                               \
    } else {                                                                   \
      *(ok) = 0;                                                               \
    }                                                                          \
  } while (0)

#define arrayExtend(array, n_items, N)                                         \
  do {                                                                         \
    bool ok = false;                                                           \
    arrayTryExtend(array, n_items, N, &ok);                                    \
    runtimeAssert(ok, "arrayTryExtend() failed");                              \
  } while (0)

#define arrayRemoveAt(array, index)                                            \
  do {                                                                         \
    if ((array)->len > 1 && (index) < (array)->len) {                          \
      for (usize i = (index); i < (array)->len - 1; i++) {                     \
        (array)->items[i] = (array)->items[i + 1];                             \
      }                                                                        \
      memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items);    \
      (array)->len--;                                                          \
    }                                                                          \
  } while (0)

#define arrayReverse(T, array)                                                 \
  do {                                                                         \
    if ((array)->len > 1) {                                                    \
      usize front = 0, back = (array)->len - 1;                                \
      while (front < back) {                                                   \
        T tmp = (array)->items[front];                                         \
        (array)->items[front] = (array)->items[back];                          \
        (array)->items[back] = tmp;                                            \
        front++, back--;                                                       \
      }                                                                        \
    }                                                                          \
  } while (0)

#define arrayTryAppendAt(array, idx, item, ok)                                 \
  do {                                                                         \
    if ((idx) < (array)->len) {                                                \
      if ((array)->cap - (array)->len <= 1) {                                  \
        arrayTryResize(array, (array)->cap + MISC_ARRAY_RESERVE, ok);          \
        if (!*(ok))                                                            \
          break;                                                               \
      }                                                                        \
      memmove((array)->items + ((idx) + 1), (array)->items + (idx),            \
              ((array)->len - (idx)) * sizeof *(array)->items);                \
      (array)->items[(idx)] = (item);                                          \
      (array)->len++;                                                          \
      *(ok) = 1;                                                               \
    } else {                                                                   \
      arrayTryAppend(array, item, ok);                                         \
    }                                                                          \
  } while (0)

#define arrayAppendAt(array, idx, item)                                        \
  do {                                                                         \
    bool ok = false;                                                           \
    arrayTryAppendAt(array, idx, item, &ok);                                   \
    runtimeAssert(ok, "arrayTryAppend() failed");                              \
  } while (0)

#define arrayFree(array)                                                       \
  do {                                                                         \
    bool ok = false;                                                           \
    arrayTryResize(array, 0, &ok);                                             \
    (void)ok;                                                                  \
  } while (0)

typedef struct {
  char *items;
  usize cap, len;
} String;

typedef struct {
  const char *items;
  usize len;
} StringView;

// Exclusive
#define sliceInit(slice, ptr, length, begin, end)                              \
  do {                                                                         \
    if ((ptr) == NULL || (begin) > (end))                                      \
      break;                                                                   \
    usize _b, _e;                                                              \
    _b = (begin) > (length) ? (length) : (begin);                              \
    _e = (end) > (length) ? (length) : (end);                                  \
    (slice)->items = (ptr) + (_b);                                             \
    (slice)->len = ((_e) - (_b));                                              \
  } while (0)

#define sliceInitFromArray(slice, array, begin, end)                           \
  sliceInit(slice, (array)->items, (array)->len, begin, end)

#define stringFmt(s) (int)(s).len, (s).items
bool viewSplitBy(StringView *sv, const char *delims, StringView *out);
StringView viewTrimStartBy(StringView *sv, const char *delims);
StringView viewTrimEndBy(StringView *sv, const char *delims);
StringView viewTrimBy(StringView *sv, const char *delims);

void stringToUpper(String *string);
void stringToLower(String *string);
String stringReadFile(FILE *file);
String stringReadPath(const char *path);
String stringPrintf(const char *fmt, ...);
char *tmpPrintf(Arena *arena, const char *fmt, ...);
#define cstringPrintf(fmt, ...) stringPrintf(fmt, __VA_ARGS__).items

#ifdef MISC_IMPL

static bool isDelimsMatch(char target, const char *delims) {
  for (usize i = 0; i < strlen(delims); i++) {
    if (delims[i] == target)
      return true;
  }
  return false;
}

StringView viewTrimStartBy(StringView *sv, const char *delims) {
  StringView result = {0};
  usize i = 0;

  if (sv->len < 1)
    goto end;

  while (i < sv->len && isDelimsMatch(sv->items[i], delims))
    i++;

  result.items = sv->items + i;
  result.len = sv->len - i;

end:
  return result;
}

StringView viewTrimEndBy(StringView *sv, const char *delims) {
  StringView result = {0};
  usize i;

  if (sv->len < 1)
    goto end;

  i = sv->len - 1;
  while (i > 0 && isDelimsMatch(sv->items[i], delims))
    i--;

  result.items = sv->items;
  if (i == 0)
    result.len = 0;
  else
    result.len = i + 1;

end:
  return result;
}

StringView viewTrimBy(StringView *sv, const char *delims) {
  StringView result = viewTrimStartBy(sv, delims);
  return viewTrimEndBy(&result, delims);
}

bool viewSplitBy(StringView *sv, const char *delims, StringView *out) {
  StringView result;
  usize i = 0;

  if (sv->len == 0)
    return false;

  while (i < sv->len && !isDelimsMatch(sv->items[i], delims))
    i += 1;

  result.items = sv->items, result.len = i;

  if (i < sv->len) {
    sv->len -= i + 1;
    sv->items += i + 1;
  } else {
    sv->len = 0;
    sv->items += i;
  }

  if (out != NULL)
    *out = result;

  return true;
}

void stringToUpper(String *string) {
  for (usize i = 0; i < string->len; i++) {
    if (islower(string->items[i]))
      string->items[i] = toupper(string->items[i]);
  }
}

void stringToLower(String *string) {
  for (usize i = 0; i < string->len; i++)
    if (isupper(string->items[i]))
      string->items[i] = tolower(string->items[i]);
}

String stringReadFile(FILE *file) {
  String string = {0};
  bool ok;
  long pos;

  if (ferror(file) || feof(file))
    goto end;

  if (fseek(file, 0, SEEK_END) != 0)
    goto end;

  if ((pos = ftell(file)) <= 0) {
    goto end;
  }

  rewind(file);
  arrayTryResize(&string, (usize)pos + 1, &ok);

  if (ok) {
    fread(string.items, 1, string.cap, file);
    string.len = (usize)pos;
  }

end:
  return string;
}

String stringReadPath(const char *path) {
  String result = {0};
  FILE *file = fopen(path, "r");

  if (file != NULL) {
    result = stringReadFile(file);
    fclose(file);
  }
  return result;
}

String stringPrintf(const char *fmt, ...) {
  String str = {0};
  va_list va;
  int size;

  va_start(va, fmt);
  size = vsnprintf(NULL, 0, fmt, va);
  va_end(va);

  if (size > 0) {
    arrayResize(&str, (usize)size + 1);
    va_start(va, fmt);
    vsnprintf(str.items, str.cap, fmt, va);
    va_end(va);
    str.len += size;
  }

  return str;
}

char *tmpPrintf(Arena *arena, const char *fmt, ...)
{
  char *buf = NULL;
  va_list va;
  int size;

  va_start(va, fmt);
  size = vsnprintf(NULL, 0, fmt, va);
  va_end(va);

  if (size > 0) {
    buf = arenaAlloc(arena, (usize)size + 1);
    va_start(va, fmt);
    vsnprintf(buf, (usize)size + 1, fmt, va);
    va_end(va);
  }

  return buf;
}

#endif

#define MISC_FNV_BASIS (0x811c9dc5)
#define MISC_FNV_PRIME (0x01000193)

#ifndef MISC_MAP_LOADF
#define MISC_MAP_LOADF (0.55)
#else
#if MISC_MAP_LOADF >= 1.0
#error Load factor must be less than 1.0
#endif
#endif

#ifndef MISC_MAP_MINIMUM
#define MISC_MAP_MINIMUM (8)
#endif

/*

A proper Hash map with open addressing, inspired from the book
`Crafting interpreters`.

This hashmap uses void* as a mean to do generic.
Let K and V be a type of key and value respectively.
On such function, one must provide a K* and V* as an argument,
each with the size.

K* and V* will be DEEP COPIED from caller into the table.
If user trying to get a value from a said table, the user must NOT
at ANY circumstances, calling free() on the return value directly.
That would resulting in double free after the call of map_free().

K* and V* value from the table will be freed when:
1. Call of map_delete()
2. Call of map_free()

map_free() will free all the backing memory of K* and V* thoroughly
until N capacity of table. map_delete() will only free
1 entry of K and V, marking it as tombstone and can be used again
if needed.

*/

typedef struct {
  void *key, *value;
  u32 hash, keySize;
} HashEntry;

typedef struct {
  const void *key, *value;
  usize keySize, pos;
} HashPair;

typedef struct {
  HashEntry *items;
  usize cap, len;
} HashMap;

u32 hashFNV32(const void *ptr, usize size);
bool hashMapInit(HashMap *map, usize capacity);
bool hashMapDelete(HashMap *map, const void *key, usize keySize);
void hashMapFree(HashMap *map);
void *hashMapGet(HashMap *map, const void *key, usize keySize);
bool hashMapIterate(HashMap *map, HashPair *input);
bool hashMapPut(HashMap *map, const void *key, usize keySize, const void *value,
                usize valueSize);

#ifdef MISC_IMPL
#define hashMapLoadFactor(map) ((f64)(map)->len / (f64)(map)->cap)

bool hashMapInit(HashMap *map, usize capacity) {
  bool ok;
  memset(map, 0, sizeof *map);
  arrayTryResize(map, alignSizeUp(capacity), &ok);
  return ok;
}

static inline bool compareKey(HashEntry *dst, const void *key, usize keySize,
                              u32 hash) {
  return dst->keySize == keySize && dst->hash == hash &&
         memcmp(dst->key, key, keySize) == 0;
}

static HashEntry *hashEntryFind(HashMap *map, const void *key, usize keySize,
                                u32 hash) {
  usize idx = hash % map->cap;
  HashEntry *tombstone = NULL;

  while (true) {
    HashEntry *entry = &map->items[idx];
    if (entry->key == NULL) {
      if (entry->value == NULL) {
        return tombstone != NULL ? tombstone : entry;
      } else {
        if (tombstone == NULL)
          tombstone = entry;
      }
    } else if (compareKey(entry, key, keySize, hash)) {
      return entry;
    }
    idx = (idx + 1) % map->cap;
  }
}

static bool hashMapGrow(HashMap *map, usize into) {
  HashMap newer = {0};
  bool ok;

  arrayTryResize(&newer, into, &ok);
  if (!ok)
    return ok;

  for (usize i = 0; i < map->cap; i++) {
    HashEntry *entry, *dest;
    entry = &map->items[i];

    if (entry->key == NULL)
      continue;

    dest = hashEntryFind(&newer, entry->key, entry->keySize, entry->hash);
    *dest = *entry;
    newer.len += 1;
  }

  arrayFree(map);
  *map = newer;
  return true;
}

bool hashMapPut(HashMap *map, const void *key, usize keySize, const void *value,
                usize valueSize) {
  HashEntry *entry;
  u32 hash;
  bool isNewKey, isNewPull, ret = false;

  if (map->cap < MISC_MAP_MINIMUM) {
    if (!hashMapInit(map, MISC_MAP_MINIMUM))
      goto end;
  }

  if (hashMapLoadFactor(map) >= MISC_MAP_LOADF) {
    if (!hashMapGrow(map, map->cap * 2))
      goto end;
  }

  hash = hashFNV32(key, keySize);
  entry = hashEntryFind(map, key, keySize, hash);
  isNewKey = entry->key == NULL;
  isNewPull = isNewKey && entry->value == NULL;

  if (isNewKey) {
    if ((entry->key = alignedAlloc(keySize, 1)) != NULL) {
      if ((entry->value = alignedAlloc(valueSize, 1)) == NULL) {
        free(entry->key);
        goto end;
      }
    } else {
      goto end;
    }

    entry->keySize = keySize;
    entry->hash = hash;
    memmove(entry->key, key, keySize);

    if (isNewPull)
      map->len++;
  }

  memmove(entry->value, value, valueSize);
  ret = true;

end:
  return ret;
}

void *hashMapGet(HashMap *map, const void *key, usize keySize) {
  u32 hash = hashFNV32(key, keySize);
  HashEntry *entry = hashEntryFind(map, key, keySize, hash);

  if (entry->key != NULL)
    return entry->value;

  return NULL;
}

bool hashMapDelete(HashMap *map, const void *key, usize keySize) {
  u32 hash = hashFNV32(key, keySize);
  HashEntry *entry = hashEntryFind(map, key, keySize, hash);

  if (entry->key == NULL)
    return false;

  free(entry->key);
  free(entry->value);
  memset(entry, 0, sizeof *entry);
  entry->value = (void *)0xdead;
  map->len--;
  return true;
}

void hashMapFree(HashMap *map) {
  for (usize i = 0; i < map->cap; i++) {
    HashEntry entry = map->items[i];
    if (entry.key == NULL || (uintptr_t)entry.value == 0xdead)
      continue;

    free(entry.key);
    free(entry.value);
  }
  arrayFree(map);
}

bool hashMapIterate(HashMap *map, HashPair *input) {
  for (; input->pos < map->cap; input->pos++) {
    HashEntry entry = map->items[input->pos];

    if (entry.key != NULL) {
      input->key = entry.key;
      input->value = entry.value;
      input->keySize = entry.keySize;
      input->pos++;
      return true;
    }
  }

  input->pos = 0;
  return false;
}

u32 hashFNV32(const void *ptr, usize size) {
  const u8 *bytes = ptr;
  u32 baseValue = MISC_FNV_BASIS;
  for (usize i = 0; i < size; i++) {
    baseValue ^= bytes[i];
    baseValue = (baseValue * MISC_FNV_PRIME) & 0xFFFFFFFF;
  }
  return baseValue;
}

#endif

/*

Ring buffer, Circular buffer, Cyclic buffer.
This is a wrapper around fixed-size buffer that let you
read/write at a specific position without worried about
doing it past its size, because if it does, it'll wrap
around to position 0 instead of going past the size of buffer.

[ H, e, l, l, o, 0x0, 0x0, 0x0 ]
  ↑               ↑
 read position   write position

[ l, d, !, \n, 0x0, W, o, r ]
                    ↑
                read/write position

*/

typedef struct {
  void *buffer;
  usize write_pos, read_pos, len;
} RingBuffer;

RingBuffer ringBufferNew(void *buffer /* assume aligned */, usize len);
usize ringBufferWrite(RingBuffer *rb, const void *src, usize len);
usize ringBufferRead(RingBuffer *rb, void *dst, usize len);
void ringBufferClear(RingBuffer *rb);

#ifdef MISC_IMPL
RingBuffer ringBufferNew(void *buffer, usize len) {
  return (RingBuffer){
      .buffer = buffer,
      .len = len,
      .write_pos = 0,
      .read_pos = 0,
  };
}

usize ringBufferWrite(RingBuffer *rb, const void *src, usize len) {
  const u8 *repr = src;
  u8 *buf = rb->buffer;
  usize i;

  for (i = 0; i < len; i++, rb->write_pos = (rb->write_pos + 1) % rb->len)
    buf[rb->write_pos] = repr[i];

  return i;
}

usize ringBufferRead(RingBuffer *rb, void *dst, usize len) {
  u8 *repr = dst;
  const u8 *buf = rb->buffer;
  usize i;

  for (i = 0; i < len; i++, rb->read_pos = (rb->read_pos + 1) % rb->len) {
    repr[i] = buf[rb->read_pos];
  }

  return i;
}

void ringBufferClear(RingBuffer *rb) {
  memset(rb->buffer, 0, rb->len);
  rb->write_pos = 0;
  rb->read_pos = 0;
}

#endif

double dataSetAverage(const double *ds, const usize N);
double dataSetMedian(const double *ds, const usize N);
double dataSetRange(const double *sortedDs, const usize N);
double dataSetMode(const double *ds, const usize N);
void dataSetSort(double *ds, const usize N);

#ifdef MISC_IMPL

static int dataSetCompare(const void *_lhs, const void *_rhs) {
  const double *lhs = _lhs, *rhs = _rhs;

  return *lhs > *rhs ? -1 : *lhs == *rhs ? 0 : 1;
}

void dataSetSort(double *ds, const usize N) {
  if (N > 0)
    qsort(ds, N, sizeof *ds, dataSetCompare);
}

double dataSetAverage(const double *ds, const usize N) {
  double avg = 0;
  for (usize i = 0; i < N; i++)
    avg += ds[i];

  return N > 0 ? avg / N : 0;
}

double dataSetMedian(const double *ds, const usize N) {
  double med = 0;
  if (!N)
    goto end;
  else if (N % 2 != 0)
    med = ds[N / 2];
  else
    med = dataSetAverage(&ds[N / 2 - 1], 2);

end:
  return med;
}

double dataSetRange(const double *sortedDs, const usize N) {
  if (!N)
    return 0.0;
  return sortedDs[N - 1] - sortedDs[0];
}

double dataSetMode(const double *ds, const usize N) {
  HashMap hm;
  HashPair pair = {0};
  double mod = 0.0;
  usize most = 0;

  if (!N)
    goto end;

  hashMapInit(&hm, 8);

  for (usize i = 0; i < N; i++) {
    usize *count = hashMapGet(&hm, &ds[i], sizeof *ds), init = 1;

    if (!count)
      hashMapPut(&hm, &ds[i], sizeof *ds, &init, sizeof init);
    else
      *count += 1;
  }

  while (hashMapIterate(&hm, &pair)) {
    const double *key = pair.key;
    const usize *val = pair.value;

    if (*val > most) {
      most = *val;
      mod = *key;
    }
  }

  hashMapFree(&hm);
end:
  return mod;
}

#endif

#endif

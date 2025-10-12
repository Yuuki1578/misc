/*

Copyright (c) 2025 Awang Destu Pradhana <destuawang@gmail.com>
Licensed under MIT License. All right reserved.

######          ######   ###     #############         ###############
######          ######   ###   ################     ###################
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

#define MISC_WORD_SIZE (sizeof(void *))

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#ifndef _LARGE_FILE
#define _LARGE_FILE
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <threads.h>

#if __STDC_VERSION__ >= 201700L

#define MISC_SWAP(Lhs, Rhs)          \
  do {                               \
    typeof((Lhs)) LooseCopy = (Lhs); \
    (Lhs) = (Rhs);                   \
    (Rhs) = LooseCopy;               \
  } while (0)

#endif

#if defined(__unix__) || defined(__linux__)

#define MISC_POSIX_HOST

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#ifdef MISC_POSIX_HOST

#include <sys/mman.h>

#define MISC_BUILTIN_ALLOC(Size) mmap(NULL, Size, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0)
#define MISC_BUILTIN_FREE(Ptr, Size) munmap(Ptr, Size)
#define MISC_BUILTIN_ALLOC_FAIL MAP_FAILED

#else

#define MISC_BUILTIN_ALLOC(Size) malloc(Size)
#define MISC_BUILTIN_FREE(Ptr, Size) free(Ptr)
#define MISC_BUILTIN_ALLOC_FAIL NULL

#endif

#ifdef MISC_POSIX_HOST
#include <alloca.h>
#define MISC_STACK_ALLOC(Size) alloca(Size)
#endif

/* ===== ARENA SECTION ===== */
#define ARENA_PAGE (1ULL << 12ULL)
#define REMAIN_OF(Arena) ((Arena)->Total - (Arena)->Offset)

/* Arena types, a single linked list that point to the next allocator.
For a better portability, we using uint8_t* instead of raw void* on a pointer
arithmetic context.

You create the arena using arenaCreate() that accept 1 arguments, the initial
size.

The linked list is made of 4 members, described as follow:
1. next, is the next allocator that have 2 states, null or nonnull.
2. data, is a raw pointer from a libc allocator casted to uint8_t*.
3. total, is the total size of memory region this allocator hold.
4. offset, is the difference between base address and current address.
   This member is always incremented on a call of arena_alloc() or
   arenaRealloc()

When you call arenaAlloc(), it'll check if the current allocator have memory
as large as "size". If it is, it return the current memory region as a chunk of
memory

The chunk is a result of difference between base address (data) and offset,
for example:

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
   ^
 offset

Here we have a 5 worth of bytes with each of respective memory addresses.
If, say, we want 2 bytes chunk from that region, we can do:

  chunk = buffer + offset
  offset += size
  return chunk

Here, we add a difference between buffer and offset, but right now the offset
is zero, so the difference is 5. Now you have a 2 possible outcome:
1. A chunk worth of 2 bytes.
2. A chunk worth more than 2 bytes.

You could use the chunk as if it was 2 bytes of memory, or you could use it as a
chunk of 2 bytes or more, up to 0x4.

Now if you look at our allocator again, things would change

[ 0x0, 0x1, 0x2, 0x3, 0x4 ] <- buffer
             ^
           offset

Ok cool, now we have 3 bytes left, starting at address 0x2.
Now, say, you want more than 2 bytes, what if 5 bytes? can you?

Remember that arenaAlloc() will check if the current allocator have enough
chunk for us to take? When the remaining bytes (total - offset) is not enough,
arenaAlloc() will create a new allocator, pointed by @Next

The chunk returned by arenaAlloc() or arenaRealloc() may be NULL, so you must
check it before using it.

Now for the best part is that we only need to free our arena's once and we're
done with it. Other neat thing is that you can use the arena and pass it around
to a bunch of function, so that you know that those section of your program need
to allocate some memory.

NOTE:
Since the arena is just a linked list, it search for a suitable arena to perform
allocation by lineary check if the arena is match the requirement (the size).
So it would be better to create a big chunk of arena. The default one is
ARENA_PAGE or 4096 bytes (same as in my system, see the getpagesize(2)), which
is enough for everyone nowadays. */

typedef struct Arena Arena;

struct Arena {
  Arena *Next;
  size_t Total, Offset;
  uint8_t *Data;
};

#ifdef MISC_USE_GLOBAL_ALLOCATOR
#define MISC_GLOBAL_ALLOCATOR_PAGING (ARENA_PAGE * 4)
static Arena *MiscGlobalAllocator = NULL;
#endif

static inline Arena *arenaCreate(size_t Size) {
  if (Size < 1)
    return NULL;

  Arena *HeadNode = (Arena *)MISC_BUILTIN_ALLOC(sizeof *HeadNode + Size);
  if (HeadNode == MISC_BUILTIN_ALLOC_FAIL)
    return NULL;

  HeadNode->Next = NULL;
  HeadNode->Total = Size;
  HeadNode->Offset = 0;
  HeadNode->Data = ((uint8_t *)HeadNode) + sizeof *HeadNode;
  return HeadNode;
}

static inline Arena *findSuitableArena(Arena *BaseArena, size_t Size, int *Found) {
  Arena *Visitor = BaseArena, *LastNonnull;
  while (Visitor) {
    if (REMAIN_OF(Visitor) >= Size) {
      *Found = 1;
      return Visitor;
    }

    LastNonnull = Visitor;
    Visitor = Visitor->Next;
  }

  *Found = 0;
  return LastNonnull;
}

static inline void *arenaAlloc(Arena *Input, size_t Size) {
  Arena *Suitable;
  int Found;
  size_t SizeRequired;

  if (!Input || !Size)
    return NULL;

  Suitable = findSuitableArena(Input, Size, &Found);

  /*
  TODO:
  1. Special page for @Size > @Total
  2. Dataset member to indicating a special page
  */
  SizeRequired = Size > Suitable->Total ? Size * 2 : Suitable->Total;

  if (!Found) {
    Suitable->Next = arenaCreate(SizeRequired);
    if (!Suitable->Next)
      return NULL;

    Suitable = Suitable->Next;
  }

  void *Result = Suitable->Data + Suitable->Offset;
  Suitable->Offset += Size;
  return Result;
}

static inline void *arenaRealloc(Arena *Input, void *TargetPtr, size_t OldSize, size_t NewSize) {
  void *Result = arenaAlloc(Input, NewSize);
  if (!Result)
    return NULL;

  else if (!TargetPtr)
    return Result;

  memcpy(Result, TargetPtr, OldSize > NewSize ? NewSize : OldSize);
  return Result;
}

static inline void arenaFree(Arena *Input) {
  while (Input) {
    Arena *Temporary = Input->Next;
    size_t Total = Input->Total;
    MISC_BUILTIN_FREE(Input, Total);
    Input = Temporary;
  }
}

#ifndef MISC_USE_GLOBAL_ALLOCATOR

#ifndef MISC_ALLOC
#define MISC_ALLOC(Size) malloc((Size))
#endif

#ifndef MISC_CALLOC
#define MISC_CALLOC(Count, Size) calloc((Count), (Size))
#endif

#ifndef MISC_REALLOC
#define MISC_REALLOC(Ptr, OldSize, NewSize) realloc((Ptr), (NewSize))
#endif

#ifndef MISC_FREE
#define MISC_FREE(Ptr) free((Ptr))
#endif

#else

/*
IMPORTANT: This macro shall be called in every main function.
REQUIRED_MACRO: @MISC_USE_GLOBAL_ALLOCATOR
*/
#define ARENA_INIT()                                                 \
  do {                                                               \
    MiscGlobalAllocator = arenaCreate(MISC_GLOBAL_ALLOCATOR_PAGING); \
  } while (0)

#define ARENA_DROP()                \
  do {                              \
    arenaFree(MiscGlobalAllocator); \
    MiscGlobalAllocator = NULL;     \
  } while (0)

#define MISC_ALLOC(Size) arenaAlloc(MiscGlobalAllocator, (Size))
#define MISC_CALLOC(Count, Size) arenaAlloc(MiscGlobalAllocator, (Count) * (Size))
#define MISC_REALLOC(Ptr, OldSize, NewSize) arenaRealloc(MiscGlobalAllocator, (Ptr), (OldSize), (NewSize))
#define MISC_FREE(Ptr) arenaFree(MiscGlobalAllocator)

#endif
/* ===== ARENA SECTION ===== */

/* ===== VECTOR SECTION ===== */
#ifndef VECTOR_ALLOC_FREQ
#define VECTOR_ALLOC_FREQ ((8ULL) * MISC_WORD_SIZE)
#endif

#define vectorPushMany(Vector, ...) vectorPushManyFn(Vector, __VA_ARGS__, NULL)

/* Struct Vector, is a dynamicaly allocated structure that behave similar like
array, it's items is stored in a contigous manner and cache-efficient. However,
the operation like appending the vector with new item or getting the specific
item at a certain location may be computed at runtime, because of how the struct
works.

Vector can work surprisingly well using the following requirements:
1. The vector must know the size for each item.

And the rest of such metadata is stored automatically within the vector like
capacity and length.

It might be slow compared to functionality defined in <libmis/list.h> because
task like appending/getting the item requiring the vector to copy each bytes
from the specified item.

But that cons is paid off because it can store almost anything you can imagine.
*/

typedef struct {
  uint8_t *Items;
  size_t ItemSize;
  size_t Length;
  size_t Capacity;
} Vector;

static inline Vector vectorCreateWith(size_t InitialCapacity, size_t ItemSize) {
  Vector NewVector = {
      .Items = NULL,
      .ItemSize = ItemSize,
  };

  if (ItemSize == 0)
    return (Vector){0};
  else if (InitialCapacity == 0)
    return NewVector;

  NewVector.Items = (uint8_t *)MISC_CALLOC(InitialCapacity, ItemSize);
  if (NewVector.Items == 0)
    return NewVector;
  else
    NewVector.Capacity = InitialCapacity;

  return NewVector;
}

static inline Vector vectorCreate(size_t ItemSize) {
  /* Inherit */
  return vectorCreateWith(0, ItemSize);
}

static inline bool vectorResize(Vector *Input, size_t Into) {
  uint8_t *Temporary;

  if (Input == NULL || Input->Capacity == Into || Input->ItemSize == 0)
    return false;

  Temporary = (uint8_t *)MISC_REALLOC(Input->Items, Input->ItemSize * Input->Capacity, Input->ItemSize * Into);
  if (Temporary == 0)
    return false;

  Input->Items = Temporary;
  Input->Capacity = Into;
  Input->Length = Into < Input->Length ? Into : Input->Length;

  return true;
}

static inline bool vectorMakeFit(Vector *Input) {
  return vectorResize(Input, Input != NULL ? Input->Length : 0);
}

static inline size_t vectorRemaining(Vector *Input) {
  if (Input != NULL)
    return Input->Capacity - Input->Length;
  else
    return 0;
}

static inline void *vectorGet(Vector *Input, size_t Index) {
  if (Input != NULL && Index < Input->Length) {
    if (Input->Capacity > 0)
      return (Input->Items + (Input->ItemSize * Index));
  }
  return NULL;
}

static inline void *vectorGetPosition(Vector *Input, size_t Index) {
  if (Input != NULL && Index < Input->Capacity)
    return (Input->Items + (Input->ItemSize * Index));

  return NULL;
}

static inline void vectorPush(Vector *Input, void *AnyData) {
  uint8_t *IncrementPtr;

  if (Input == NULL || AnyData == NULL)
    return;

  if (Input->Capacity == 0) {
    if (!vectorResize(Input, VECTOR_ALLOC_FREQ))
      return;

  } else if (vectorRemaining(Input) <= 1) {
    if (!vectorResize(Input, Input->Capacity * 2))
      return;
  }

  IncrementPtr = Input->Items + (Input->ItemSize * Input->Length++);
  memcpy(IncrementPtr, AnyData, Input->ItemSize);
}

static inline void vectorPushManyFn(Vector *Input, ...) {
  va_list VaList;
  void *Arguments;

  if (Input == NULL)
    return;

  va_start(VaList, Input);

  while ((Arguments = va_arg(VaList, void *)) != NULL)
    vectorPush(Input, Arguments);

  va_end(VaList);
}

static inline void vectorFree(Vector *Input) {
  if (Input != NULL) {
    if (Input->Items != NULL)
#ifndef MISC_USE_GLOBAL_ALLOCATOR
      MISC_FREE(Input->Items);
#else
      Input->Items = NULL;
#endif

    Input->Items = 0;
    Input->Capacity = 0;
    Input->Length = 0;
  }
}
/* ===== VECTOR SECTION ===== */

/* ===== STRING SECTION ===== */
#ifndef CSTR
#define CSTR(String) ((char *)(String.Buffer.Items))
#endif

#define stringPushMany(String, ...) stringPushManyFn(String, __VA_ARGS__, '\0')

#define stringPushStrMany(String, ...) \
  stringPushStrManyFn(String, __VA_ARGS__, NULL)

typedef struct {
  Vector Buffer;
} String;

static inline String stringCreateWith(size_t InitCapacity) {
  return (String){vectorCreateWith(InitCapacity, 1)};
}

static inline String stringCreate(void) {
  /* Inherit */
  return stringCreateWith(0);
}

static inline void stringPush(String *Input, char Character) {
  /* Inherit */
  vectorPush((Vector *)Input, &Character);
}

static inline void stringPushManyFn(String *Input, ...) {
  va_list VaList;
  va_start(VaList, Input);
  vectorPushMany((Vector *)Input, VaList);
  va_end(VaList);
}

static inline void stringPushStr(String *Input, char *Cstring) {
  size_t Length;

  if (Cstring == NULL)
    return;

  Length = strlen(Cstring);
  while (Length--)
    stringPush(Input, *Cstring++);
}

static inline void stringPushStrManyFn(String *Input, ...) {
  va_list VaList;
  char *Cstring;

  va_start(VaList, Input);
  while ((Cstring = va_arg(VaList, char *)) != NULL)
    stringPushStr(Input, Cstring);

  va_end(VaList);
}

#ifndef MISC_USE_GLOBAL_ALLOCATOR
static inline void stringFree(String *Input) {
  /* Inherit */
  vectorFree((Vector *)Input);
}
#endif

static inline String stringFromStr(char *Cstring, size_t Length) {
  String NewString;

  if (Cstring == NULL || Length < 1)
    return stringCreate();

  NewString = stringCreateWith(Length + 1);
  stringPushStr(&NewString, Cstring);
  return NewString;
}
/* ===== STRING SECTION ===== */

/* ===== REFCOUNT SECTION ===== */
/* This is the implementation of a reference counting
originally https://github.com/jeraymond/refcount.git

By default, when you allocate something using malloc() or MISC_REALLOC(), the
object is gone by the time you call free(), but in reference counting, that's
not the case.

You see, if you use reference counting, you can increase it's lifetime count
(Strong), or make it dying (Weak). If the object's lifetime count is zero, the
object is released, however if the object count is still strong, the routine to
make it weak, which is refcount_weak(), is only decrease the object's lifetime
count by 1.

When you make the object strong/weak, there is a guard to make the count only
change in one thread, so that the count doesn't get messed up when you use the
object on a multi-threaded environment, and that guard is called a mutex.

Before you increase/decrease the count, the routine will check if its possible
to obtain a lock of a mutex. If it is, the mutex is locked and the current
thread on which this routine is being called will wait until the mutex is
unlocked. This way you don't encounter data race.

WARNING
Don't ever free() the reference counted object manually, if you want to release
it all, just use refcount_drop(). */

typedef struct {
  mtx_t Mutex;
  void *RawData;
  size_t Count;
} RefCount;

static inline bool refcountLock(mtx_t *Mutex) {
  switch (mtx_trylock(Mutex)) {
  case thrd_error:
    return false;

  case thrd_busy:
    if (mtx_unlock(Mutex) != thrd_success)
      return false;

    if (mtx_trylock(Mutex) != thrd_success)
      return false;
  }

  return true;
}

static inline void *getRefcount(void *Object) {
  const uint8_t *MemoryPadding = (uint8_t *)Object;
  return (void *)(MemoryPadding - sizeof(RefCount));
}

static inline void *refcountAlloc(size_t Size) {
  RefCount *ObjectTemplate;
  uint8_t *SliceMemory;

  if ((ObjectTemplate = (RefCount *)calloc(1, sizeof *ObjectTemplate + Size)) ==
      NULL)
    return NULL;

  if (mtx_init(&ObjectTemplate->Mutex, mtx_plain) != thrd_success) {
    free(ObjectTemplate);
    return NULL;
  }

  ObjectTemplate->Count = 1;
  SliceMemory = (uint8_t *)ObjectTemplate;
  SliceMemory = SliceMemory + sizeof *ObjectTemplate;
  ObjectTemplate->RawData = SliceMemory;

  return (void *)SliceMemory;
}

static inline bool refcountUpgrade(void **Object) {
  RefCount *PaddedBytes;
  if (Object == NULL || *Object == NULL)
    return false;

  PaddedBytes = (RefCount *)getRefcount(*Object);
  if (refcountLock(&PaddedBytes->Mutex)) {
    PaddedBytes->Count++;
    mtx_unlock(&PaddedBytes->Mutex);
    return true;
  }

  return false;
}

static inline bool refcountDegrade(void **Object) {
  RefCount *PaddedBytes;
  bool MarkAsFree;

  if (Object == NULL || *Object == NULL)
    return false;

  PaddedBytes = (RefCount *)getRefcount(*Object);
  MarkAsFree = false;

  if (refcountLock(&PaddedBytes->Mutex)) {
    if (PaddedBytes->Count == 1)
      MarkAsFree = true;
    else
      PaddedBytes->Count--;

    mtx_unlock(&PaddedBytes->Mutex);
  } else {
    return false;
  }

  if (MarkAsFree) {
    mtx_destroy(&PaddedBytes->Mutex);
    free(PaddedBytes);
    *Object = NULL;
  }

  return true;
}

static inline void refcountDrop(void **Object) {
  while (refcountDegrade(Object))
    ;
}

static inline size_t refcountLifetime(void **Object) {
  RefCount *PaddedBytes;
  size_t ObjectLifetime;

  if (Object == NULL || *Object == NULL)
    return 0;

  PaddedBytes = (RefCount *)getRefcount(*Object);
  if (refcountLock(&PaddedBytes->Mutex)) {
    ObjectLifetime = PaddedBytes->Count;
    mtx_unlock(&PaddedBytes->Mutex);
  } else {
    return 0;
  }

  return ObjectLifetime;
}
/* ===== REFCOUNT SECTION ===== */

/* ===== LIST SECTION ===== */
#if __STDC_VERSION__ >= 201700L
#ifndef MISC_LIST_FREQ
#define MISC_LIST_FREQ ((8ULL) * MISC_WORD_SIZE)
#endif

/* NOTE:
The macros below are works on all struct implementing the field listed here:
1. <Type> *items.
2. size_t capacity.
3. size_t length.

Where <Type> are some concrete types. */

/* Anonymous struct, a generic-like type for C.
Examples:

    int main(void)
    {
        List(int) some_integer = {0};
        List(List(char*)) some_list_of_list_of_string = {0};

        list_append(some_integer, 10);
    }

*/
#define List(T)      \
  struct {           \
    T *Items;        \
    size_t Capacity; \
    size_t Length;   \
  }

/* Resizing the list up and/or down. */
#define listResize(List, Size)                                             \
  do {                                                                     \
    if ((List).Capacity < 1 || (List).Items == NULL) {                     \
      (List).Items = (typeof((List).Items))MISC_CALLOC(                    \
          (Size), sizeof(typeof(*(List).Items)));                          \
      if ((List).Items == NULL)                                            \
        break;                                                             \
    } else {                                                               \
      typeof((List).Items) Temporary = (typeof((List).Items))MISC_REALLOC( \
          (List).Items, (List).Capacity * sizeof(*(List).Items),           \
          (Size) * sizeof(*(List).Items));                                 \
      if (Temporary == NULL)                                               \
        break;                                                             \
      (List).Items = Temporary;                                            \
    }                                                                      \
    if ((Size) < (List).Length)                                            \
      (List).Length = Size;                                                \
    (List).Capacity = Size;                                                \
  } while (0)

/* Make the list fitting to it's length. */
#define listMakeFit(List)                                      \
  do {                                                         \
    if ((List).Length < 1 || (List).Capacity == (List).Length) \
      break;                                                   \
    listResize(List, (List).Length);                           \
  } while (0)

/* Append an item to the list, increasing it's length. */
#define listAppend(List, Item)                                     \
  do {                                                             \
    if ((List).Capacity < 1) {                                     \
      listResize(List, MISC_LIST_FREQ);                            \
    } else if ((List).Capacity - (List).Length <= 1) {             \
      listResize(List, (List).Capacity * 2);                       \
    }                                                              \
    (List).Items[(List).Length++] = (typeof(*(List).Items))(Item); \
  } while (0)

/* Freeing the list, truncating it's capacity to zero. */
#ifndef MISC_USE_GLOBAL_ALLOCATOR
#define listFree(List)                               \
  do {                                               \
    if ((List).Capacity > 0 || (List).Items != NULL) \
      MISC_FREE((List).Items);                       \
    (List).Capacity = 0;                             \
    (List).Length = 0;                               \
  } while (0)

#endif
#endif
/* ===== LIST SECTION ===== */

/* ===== FILE SECTION ===== */
static inline char *readFromFileStream(FILE *FileStream) {
  int64_t OffsetMax = 0;

#if defined(_WIN32) || defined(_WIN64)
  _fseeki64(FileStream, 0, SEEK_END);
  OffsetMax = _ftelli64(file);
  _fseeki64(FileStream, 0, SEEK_SET);
#elif defined(MISC_POSIX_HOST)
  fseeko(FileStream, 0, SEEK_END);
  OffsetMax = ftello(FileStream);
  fseeko(FileStream, 0, SEEK_SET);
#else
#error Platform is not supported
#endif

  if (OffsetMax > 0) {
    char *Buffer = (char *)MISC_ALLOC(OffsetMax + 1);
    if (Buffer == NULL)
      return NULL;

    if (fread(Buffer, 1, OffsetMax, FileStream) > 0)
      return Buffer;
    else
      MISC_FREE(Buffer);
  }

  return NULL;
}

static inline char *fileRead(const char *FilePath) {
  FILE *FileStream = fopen(FilePath, "rb");
  if (FileStream != NULL) {
    char *Buffer = readFromFileStream(FileStream);
    fclose(FileStream);

    if (Buffer != NULL)
      return Buffer;
  }
  return NULL;
}

static inline char *fileReadFrom(FILE *FileStream) {
  return FileStream != NULL ? readFromFileStream(FileStream) : NULL;
}

#if defined(MISC_POSIX_HOST)

#ifndef MISC_BULK_SIZE
#define MISC_BULK_SIZE ((1 << 8) + 1)
#else
#if MISC_BULK_SIZE < 1
#undef MISC_BULK_SIZE
#define MISC_BULK_SIZE ((1 << 8) + 1)
#endif

#endif

static inline String fileDescriptorDrain(int FileDesc) {
  String NewString = stringCreateWith(BUFSIZ);
  for (;;) {
    char LocalBuffer[MISC_BULK_SIZE] = {};
    ssize_t ReadedBuffer = read(FileDesc, LocalBuffer, sizeof(LocalBuffer) - 1);

    switch (ReadedBuffer) {
    case -1:
#ifndef MISC_USE_GLOBAL_ALLOCATOR
      stringFree(&NewString);
#endif
      return stringCreate();

    case 0:
      goto EndSection;

    default:
      stringPushStr(&NewString, LocalBuffer);
      break;
    }
  }

EndSection:
  return NewString;
}

static inline size_t fileDescriptorWrite(int FileDesc, String CopyString) {
  size_t CharOffset = 0;

  for (;;) {
    char *FromOffset = (char *)vectorGet((Vector *)&CopyString, CharOffset);
    if (FromOffset == NULL)
      break;

    ssize_t FileOffset = write(FileDesc, FromOffset, MISC_BULK_SIZE - 1);
    switch (FileOffset) {
    case -1:
      return 0;

    case 0:
      goto EndSection;

    default:
      CharOffset += (size_t)FileOffset;
    }
  }

EndSection:
  return CharOffset;
}
#endif
/* ===== FILE SECTION ===== */

/* ===== LINKED LIST SECTION ===== */
#if !defined(ADDRESS_OF) && !defined(__cplusplus)
#define ADDRESS_OF(T) (&(typeof(T)){T})
#endif

#define FOR_LINK(Link, VarName)        \
  for (RawLink *VarName = (Link).Head; \
       VarName != NULL && VarName->Item != NULL; VarName = VarName->Next)

typedef struct RawLink RawLink;
struct RawLink {
  RawLink *Next;
  void *Item;
};

typedef struct {
  RawLink *Head, *Tail;
  size_t Length, ItemSize;
} LinkedList;

static inline RawLink *rawLinkCreate(void *Item, size_t Size) {
  if (Item == NULL || Size < 1)
    return NULL;

  uint8_t *Buffer = (uint8_t *)MISC_ALLOC(sizeof(RawLink) + Size);
  if (Buffer == NULL)
    return NULL;

  RawLink *NewRawLink = (RawLink *)(Buffer + 0);
  NewRawLink->Item = (void *)(Buffer + sizeof(RawLink));
  NewRawLink->Next = NULL;
  memcpy(NewRawLink->Item, Item, Size);

  return NewRawLink;
}

static inline void rawLinkAppend(RawLink **Input, void *Item, size_t Size) {
  if (Input == NULL || *Input == NULL || Item == NULL || Size < 1)
    return;

  RawLink *NewRawLink = rawLinkCreate(Item, Size);
  if (NewRawLink != NULL) {
    RawLink *CurrentLink = *Input;
    CurrentLink->Next = NewRawLink;
    *Input = NewRawLink;
  }
}

static inline RawLink *rawLinkGet(RawLink *Head, size_t Position) {
  if (Head == NULL)
    return NULL;

  while (Head->Next != NULL && Position--)
    Head = Head->Next;

  return Head;
}

static inline void *rawLinkGetItem(RawLink *Head, size_t Position) {
  RawLink *FoundNode = rawLinkGet(Head, Position);
  if (FoundNode != NULL)
    return FoundNode->Item;

  return NULL;
}

static inline void rawLinkPrepend(RawLink **Input, void *Item, size_t Size) {
  if (Input == NULL || *Input == NULL || Item == NULL || Size < 1)
    return;

  RawLink *NewRawLink = rawLinkCreate(Item, Size);
  if (NewRawLink != NULL) {
    NewRawLink->Next = *Input;
    *Input = NewRawLink;
  }
}

static inline void rawLinkFree(RawLink *Head) {
#ifndef MISC_USE_GLOBAL_ALLOCATOR
  while (Head != NULL) {
    RawLink *Next = Head->Next;
    free(Head);
    Head = Next;
  }
#else
  if (Head != NULL)
    *Head = (RawLink){};
#endif
}

static inline void linkedListAppend(LinkedList *Input, void *Item) {
  if (Input == NULL || Input->ItemSize < 1 || Item == NULL)
    return;

  if (Input->Head == NULL) {
    Input->Head = rawLinkCreate(Item, Input->ItemSize);
    if (Input->Head == NULL)
      return;

    Input->Tail = Input->Head;
    return;
  } else {
    rawLinkAppend(&Input->Tail, Item, Input->ItemSize);
  }

  Input->Length++;
}

static inline void linkedListPrepend(LinkedList *Input, void *Item) {
  if (Input == NULL || Input->ItemSize < 1 || Item == NULL)
    return;

  if (Input->Head == NULL) {
    Input->Head = rawLinkCreate(Item, Input->ItemSize);
    if (Input->Head == NULL)
      return;

    Input->Tail = Input->Head;
  } else {
    rawLinkPrepend(&Input->Head, Item, Input->ItemSize);
  }

  Input->Length++;
}

static inline void linkedListFree(LinkedList *Input) {
  if (Input != NULL) {
    rawLinkFree(Input->Head);
    Input->Head = NULL;
    Input->Tail = NULL;
    Input->Length = 0;
  }
}

#define DLINK_FORWARD (1)
#define DLINK_BACKWARD (0)

typedef struct RawDlink RawDlink;
struct RawDlink {
  RawDlink *Previous, *Next;
  void *Item;
};

static inline RawDlink *rawDlinkCreate(void *Item, size_t Size) {
  if (Item == NULL || Size < 1)
    return NULL;

  RawDlink *NewDoubleLink = NULL;
  uint8_t *BytesRoom = (uint8_t *)MISC_ALLOC(sizeof(RawDlink) + Size);

  if (BytesRoom != NULL) {
    NewDoubleLink = (RawDlink *)(BytesRoom + 0);
    NewDoubleLink->Previous = NULL;
    NewDoubleLink->Next = NULL;
    NewDoubleLink->Item = BytesRoom + sizeof(RawDlink);
    memcpy(NewDoubleLink->Item, Item, Size);
  }

  return NewDoubleLink;
}

static inline bool rawDlinkAppend(RawDlink **InputTail, void *Item, size_t Size) {
  if (InputTail == NULL || *InputTail == NULL || Item == NULL || Size < 1)
    return false;

  RawDlink *LatterDoubleLink = rawDlinkCreate(Item, Size);
  if (LatterDoubleLink != NULL) {
    RawDlink *CurrentLink = *InputTail;
    CurrentLink->Next = LatterDoubleLink;
    LatterDoubleLink->Previous = CurrentLink;
    *InputTail = LatterDoubleLink;

    return true;
  }

  return false;
}

static inline bool rawDlinkPrepend(RawDlink **InputHead, void *Item, size_t Size) {
  if (InputHead == NULL || *InputHead == NULL || Item == NULL || Size < 1)
    return false;

  RawDlink *NewHeadLink = rawDlinkCreate(Item, Size);
  if (NewHeadLink != NULL) {
    RawDlink *CurrentLink = *InputHead;
    CurrentLink->Previous = NewHeadLink;
    NewHeadLink->Next = CurrentLink;
    *InputHead = NewHeadLink;

    return true;
  }

  return false;
}

static inline RawDlink *rawDlinkRewind(RawDlink **InputTail) {
  if (InputTail == NULL || *InputTail == NULL)
    return NULL;

  while ((*InputTail)->Previous != NULL)
    (*InputTail) = (*InputTail)->Previous;

  return *InputTail;
}

static inline void rawDlinkFree(RawDlink *InputTail) {
#ifndef MISC_USE_GLOBAL_ALLOCATOR
  while (InputTail != NULL) {
    RawDlink *Previous = InputTail->Previous;
    free(InputTail);
    InputTail = Previous;
  }
#else
  if (InputTail != NULL)
    *InputTail = (RawDlink){};
#endif
}

static inline RawDlink *rawDlinkGet(RawDlink **IndependentLink, size_t Index, int SearchDirection) {
  RawDlink *Visitor = NULL;
  size_t Count = 0;

  if (IndependentLink == NULL || *IndependentLink == NULL)
    return NULL;

  Visitor = *IndependentLink;
  if (SearchDirection) {
    while (Visitor->Next != NULL && Count < Index)
      Visitor = Visitor->Next;
  } else {
    while (Visitor->Previous != NULL && Count < Index)
      Visitor = Visitor->Previous;
  }

  return Visitor;
}

static inline void *rawDlinkGetItem(RawDlink **IndependentLink, size_t Index, int SearchDirection) {
  RawDlink *LinkTarget = rawDlinkGet(IndependentLink, Index, SearchDirection);
  if (LinkTarget != NULL)
    return LinkTarget->Item;

  return NULL;
}

#define DLINK_GO_FORWARD(Dlink, VarName)                  \
  for (RawDlink *VarName = (Dlink).Head; VarName != NULL; \
       VarName = VarName->Next)

#define DLINK_GO_BACKWARD(Dlink, VarName)                 \
  for (RawDlink *VarName = (Dlink).Tail; VarName != NULL; \
       VarName = VarName->Previous)

typedef struct {
  RawDlink *Head, *Tail;
  size_t ItemSize, Length;
} DoubleLink;

static inline void doubleLinkAppend(DoubleLink *Input, void *Item) {
  if (Input == NULL || Input->ItemSize < 1 || Item == NULL)
    return;

  if (Input->Tail == NULL) {
    if ((Input->Tail = rawDlinkCreate(Item, Input->ItemSize)) == NULL)
      return;

    Input->Head = Input->Tail;
  } else {
    if (!rawDlinkAppend(&Input->Tail, Item, Input->ItemSize))
      return;
  }

  Input->Length++;
}

static inline void doubleLinkPrepend(DoubleLink *Input, void *Item) {
  if (Input == NULL || Item == NULL)
    return;

  if (Input->Tail == NULL) {
    if ((Input->Tail = rawDlinkCreate(Item, Input->ItemSize)) == NULL)
      return;

    Input->Head = Input->Tail;
  } else {
    if (!rawDlinkPrepend(&Input->Tail, Item, Input->ItemSize))
      return;

    Input->Head = Input->Tail;
  }

  Input->Length++;
}

static inline void doubleLinkFree(DoubleLink *Input) {
  if (Input != NULL) {
    rawDlinkFree(Input->Tail);
    Input->Head = NULL;
    Input->Tail = NULL;
    Input->Length = 0;
  }
}
/* ===== LINKED LIST SECTION ===== */

/* ===== PROCESS RELATED ROUTINES ===== */
#ifdef MISC_POSIX_HOST

typedef int (*ThreadTask)(void *Args);

static inline bool runSeparately(ThreadTask Routine, void *Args) {
  switch (fork()) {
  case -1:
    return false;

  case 0:
    exit(Routine(Args));

  default:
    break;
  }

  return true;
}

#endif
/* ===== PROCESS RELATED ROUTINES ===== */

/*
TODO:
1. Hash function impl.
2. Hash table impl.
3. Hash entry impl.
4. HashSet impl.

5. Win32/64 Mapping function for arena (still use malloc-family function)
6. sbrk/brk option for arena

IMPORTANT:
7. automaticaly align the memory allocated by arena (i don't think mmap does that)
*/

#endif

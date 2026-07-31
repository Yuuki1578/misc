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

#include <assert.h>
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
typedef size_t usize;
typedef float f32;
typedef double f64;

#define MISC_ALIGN (sizeof(void*))

/*

Linked list, this is designed to be used in another data structure
(See the use case on arena allocator below).

The linked list only stored the @next pointer as its member,
the value is the offset from the base pointer + sizeof NodeLink.

API:
NodeLink* initNodeLink(usize size);
    Initialize a node with the size of @size.

NodeLink* insertAfterNodeLink(NodeLink* node, usize size);
    Insert a new node after @node, preserve the newer node as
    its return value.

NodeLink* insertBeforeNodeLink(NodeLink* node, usize size);
    Insert a new node behind @node, return the node before @node.

usize lengthOfNodeLink(NodeLink* node);
    Count the length of a linked list from @node until @node is null.

void* valueOfNodeLink(NodeLink* node);
    Get the inner value from @node on its offset in memory (just a math).

void freeNodeLink(NodeLink* node);
    Free all node starting from @node.

*/

void* strictAlloc(usize size);
void* strictRealloc(void* ptr, usize size);

#define makeStack(T, ...) (&(T){__VA_ARGS__})
#define makeHeap(T, ...) memmove(strictAlloc(sizeof(T)), makeStack(T, __VA_ARGS__), sizeof(T))

#ifdef MISC_IMPL
void* strictAlloc(usize size)
{
    void* p = calloc(size, 1);
    assert(p != NULL);
    return p;
}

void* strictRealloc(void* ptr, usize size)
{
    void* p = realloc(ptr, size);
    assert(p != NULL);
    return p;
}
#endif

typedef struct NodeLink NodeLink;
struct NodeLink {
    NodeLink* next;
    // ...
};

NodeLink* initNodeLink(usize size);
NodeLink* insertAfterNodeLink(NodeLink* node, usize size);
NodeLink* insertBeforeNodeLink(NodeLink* node, usize size);
NodeLink* removeAfterNodeLink(NodeLink* node);
NodeLink* findLastNodeLink(NodeLink* node);
usize lengthOfNodeLink(NodeLink* node);
void* valueOfNodeLink(NodeLink* node);
void freeNodeLink(NodeLink* node);

#ifdef MISC_IMPL
NodeLink* initNodeLink(usize size)
{
    NodeLink* node = strictAlloc(sizeof *node + size);
    node->next = NULL;
    return node;
}

NodeLink* insertAfterNodeLink(NodeLink* node, usize size)
{
    NodeLink* next = initNodeLink(size);
    node->next = next;
    return next;
}

NodeLink* insertBeforeNodeLink(NodeLink* node, usize size)
{
    NodeLink* before = initNodeLink(size);
    before->next = node;
    return before;
}

NodeLink* removeAfterNodeLink(NodeLink* node)
{
    NodeLink* next = node->next;
    NodeLink* tmp = next != NULL ? next->next : NULL;
    node->next = tmp;
    return next;
}

NodeLink* findLastNodeLink(NodeLink* node)
{
    if (node == NULL) return NULL;
    while (node->next != NULL)
        node = node->next;

    return node;
}

void* valueOfNodeLink(NodeLink* node)
{
    return (u8*)node + sizeof *node;
}

usize lengthOfNodeLink(NodeLink* node)
{
    usize count = 0;
    while (node != NULL)
        node = node->next,
        count++;

    return count;
}

void freeNodeLink(NodeLink* node)
{
    while (node != NULL) {
        NodeLink* tmp = node->next;
        free(node);
        node = tmp;
    }
}
#endif

typedef struct Arena Arena;
#define alignUp(size) (((size) + MISC_ALIGN - 1) & ~(MISC_ALIGN - 1))

Arena* initArena(usize size);
void* allocArena(Arena* arena, usize size);
void* reallocArena(Arena* arena, void* ptr, usize sizeBefore, usize sizeAfter);
void freeArena(Arena* arena);
usize sizeOfArena(Arena* arena);

#ifdef MISC_IMPL
typedef struct {
    usize cap;
    usize len;
    // ...
} ArenaBody;

struct Arena {
    NodeLink
        *head,
        *last;
};

Arena* initArena(usize size)
{
    assert(size > 0);
    Arena* arena = strictAlloc(sizeof *arena);
    ArenaBody body = { .cap = size };
    arena->head = initNodeLink(sizeof body + size);
    arena->last = arena->head;

    ArenaBody* value = valueOfNodeLink(arena->head);
    *value = body;
    return arena;
}

void* allocArena(Arena* arena, usize size)
{
    assert(arena != NULL && size > 0);

    NodeLink* last = arena->last;
    ArenaBody* body = valueOfNodeLink(last);
    size = alignUp(size);

    if (body->cap - body->len < size) {
        usize new_size = (body->cap > size ? body->cap : size) + size;
        ArenaBody newer = { .cap = new_size };
        NodeLink* new_tail = insertAfterNodeLink(last, sizeof newer + new_size);

        arena->last = new_tail;
        last = arena->last;
        body = valueOfNodeLink(last);
        *body = newer;
    }

    void* ptr = (u8*)body + sizeof *body + body->len;
    body->len += size;
    return ptr;
}

void* reallocArena(
    Arena* arena,
    void*  ptr,
    usize  sizeBefore,
    usize  sizeAfter)
{
    if (sizeAfter == 0) return NULL;

    void* newer = allocArena(arena, sizeAfter);
    if (ptr == NULL) return newer;

    usize trueSize = sizeBefore > sizeAfter ? sizeAfter : sizeBefore;
    return memmove(newer, ptr, trueSize);
}

void freeArena(Arena* arena)
{
    if (arena != NULL) {
        freeNodeLink(arena->head);
        free(arena);
    }
}

usize sizeOfArena(Arena* arena)
{
    usize size = 0;
    if (arena == NULL) return size;

    NodeLink* node = arena->head;
    while (node != NULL) {
        ArenaBody* body = valueOfNodeLink(node);
        size += body->cap;
        node = node->next;
    }
    return size;
}
#endif

#define MISC_ARRAY_RESERVE (8)

#define Array(T)    \
    struct {        \
        T* items;   \
        usize cap;  \
        usize len;  \
    }

#define isArrayEmpty(array) ((array) != NULL ? ((array)->items == NULL && !(array)->cap) : 1)
#define remainsOfArray(array) ((array) != NULL ? ((array)->cap - (array)->len) : 0)

#define tryResizeArray(array, N, ok)                                         \
    do {                                                                     \
        if ((N) <= 0) {                                                      \
            free((array)->items);                                            \
            (array)->items = NULL;                                           \
            (array)->cap = 0;                                                \
            (array)->len = 0;                                                \
            *(ok) = 1;                                                       \
        } else {                                                             \
            void* tmp;                                                       \
            if ((array)->cap == 0) {                                         \
                tmp = calloc((N), sizeof *(array)->items);                   \
            } else {                                                         \
                tmp = realloc((array)->items, (N) * sizeof *(array)->items); \
            }                                                                \
            if (tmp != NULL) {                                               \
                *(ok) = 1;                                                   \
                (array)->items = tmp;                                        \
                (array)->cap = (N);                                          \
                if ((N) < (array)->len) {                                    \
                    (array)->len = (N);                                      \
                }                                                            \
            } else {                                                         \
                *(ok) = 0;                                                   \
            }                                                                \
        }                                                                    \
    } while (0)

#define tryAppendArray(array, item, ok)                                     \
    do {                                                                    \
        if ((array)->cap <= (array)->len) {                                 \
            tryResizeArray(array, (array)->cap + MISC_ARRAY_RESERVE, ok);   \
        }                                                                   \
        if (*(ok)) {                                                        \
            (array)->items[(array)->len++] = (item);                        \
        }                                                                   \
    } while (0)

#define tryExtendArray(array, many_ptr, N, ok)                                                \
    do {                                                                                      \
        if ((many_ptr) != NULL && (N) > 0) {                                                  \
            if (isArrayEmpty(array) || remainsOfArray(array) <= (N)) {                        \
                tryResizeArray(array, (array)->cap + (N) + MISC_ARRAY_RESERVE, ok);           \
                if (!*(ok)) {                                                                 \
                    break;                                                                    \
                }                                                                             \
            }                                                                                 \
            memmove((array)->items + (array)->len, (many_ptr), (N) * sizeof *(array)->items); \
            (array)->len += (N);                                                              \
            *(ok) = 1;                                                                        \
        } else {                                                                              \
            *(ok) = 0;                                                                        \
        }                                                                                     \
    } while (0)

#define resizeArray(array, N)          \
    do {                               \
        bool ok;                       \
        tryResizeArray(array, N, &ok); \
        assert(ok);                    \
    } while (0)

#define appendArray(array, item)          \
    do {                                  \
        bool ok;                          \
        tryAppendArray(array, item, &ok); \
        assert(ok);                       \
    } while (0)

#define extendArray(array, many_ptr, N)          \
    do {                                         \
        bool ok;                                 \
        tryExtendArray(array, many_ptr, N, &ok); \
        assert(ok);                              \
    } while (0)

#define removeArrayAt(array, index)                                               \
    do {                                                                          \
        if ((array)->len > 1 && (index) < (array)->len) {                         \
            for (usize i = (index); i < (array)->len - 1; i++) {                  \
                (array)->items[i] = (array)->items[i + 1];                        \
            }                                                                     \
            memset(&(array)->items[(array)->len - 1], 0, sizeof *(array)->items); \
            (array)->len--;                                                       \
        }                                                                         \
    } while (0)

#define shrinkArrayToFit(array) resizeArray(array, (array)->len)
#define freeArray(array) resizeArray(array, 0)

#define Slice(T)        \
    struct {            \
        const T* items; \
        usize len;      \
    }

typedef Array(char) String;
typedef Slice(char) StringView;

// Exclusive
#define initSlice(slice, ptr, length, begin, end)     \
    do {                                              \
        if ((ptr) == NULL || (begin) > (end))         \
            break;                                    \
        usize _b, _e;                                 \
        _b = (begin) > (length) ? (length) : (begin); \
        _e = (end) > (length) ? (length) : (end);     \
        (slice)->items = (ptr) + (_b);                \
        (slice)->len = ((_e) - (_b));                 \
    } while (0)

#define initSliceFromArray(slice, array, begin, end) initSlice(slice, (array)->items, (array)->len, begin, end)

#define stringFmt(s) (int)(s).len, (s).items
StringView initSvFrom(const char* cstr, usize begin, usize end);
StringView initSvFromString(String* string, usize begin, usize end);
bool splitSvBy(StringView* sv, const char* delims, StringView* out);
StringView trimStartSvBy(StringView* sv, const char* delims);
StringView trimEndSvBy(StringView* sv, const char* delims);
StringView trimSvBy(StringView* sv, const char* delims);
void toStringUppercase(String* string);
void toStringLowercase(String* string);
void reverseString(String* string);
String stringPrintf(const char* fmt, ...);
String readStreamToString(FILE* file);
String readFileToString(const char* path);
char* cstrArenaPrintf(Arena* arena, const char* fmt, ...);
char* cstrPrintf(const char* fmt, ...);

#ifdef MISC_IMPL

static bool isDelimsMatch(char target, const char* delims)
{
    for (usize i = 0; i < strlen(delims); i++) {
        if (delims[i] == target)
            return true;
    }
    return false;
}

StringView trimStartSvBy(StringView* sv, const char* delims)
{
    StringView result = {0};
    if (sv->len < 1) return result;

    usize i = 0;
    while (i < sv->len && isDelimsMatch(sv->items[i], delims))
        i++;

    result.items = sv->items + i;
    result.len = sv->len - i;
    return result;
}

StringView trimEndSvBy(StringView* sv, const char* delims)
{
    StringView result = {0};
    if (sv->len < 1) return result;

    usize i = sv->len - 1;
    while (i > 0 && isDelimsMatch(sv->items[i], delims))
        i--;

    result.items = sv->items;
    if (i == 0)
        result.len = 0;
    else
        result.len = i + 1;
    return result;
}

StringView trimSvBy(StringView* sv, const char* delims)
{
    StringView result = trimStartSvBy(sv, delims);
    return trimEndSvBy(&result, delims);
}

bool splitSvBy(
    StringView* sv,
    const char* delims,
    StringView* out)
{
    if (sv->len == 0) return false;

    usize i = 0;
    while (i < sv->len && !isDelimsMatch(sv->items[i], delims))
        i += 1;

    StringView result = {
        .items = sv->items,
        .len = i,
    };

    if (i < sv->len) {
        sv->len -= i + 1;
        sv->items += i + 1;
    } else {
        sv->len = 0;
        sv->items += i;
    }

    if (out != NULL) *out = result;
    return true;
}

StringView initSvFrom(
    const char* cstr,
    usize       begin,
    usize       end)
{
    StringView ref = {0};
    if (cstr == NULL || end < begin)
        return ref;

    usize len = strlen(cstr);
    initSlice(&ref, cstr, len, begin, end);
    return ref;
}

void toStringUppercase(String* string)
{
    for (usize i = 0; i < string->len; i++) {
        if (islower(string->items[i]))
            string->items[i] = toupper(string->items[i]);
    }
}

void toStringLowercase(String* string)
{
    for (usize i = 0; i < string->len; i++) {
        if (isupper(string->items[i]))
            string->items[i] = tolower(string->items[i]);
    }
}

void reverseString(String* string)
{
    if (string->len <= 1)
        return;

    usize front = 0,
          back = string->len - 1;

    while (front < back) {
        char tmp = string->items[front];
        string->items[front] = string->items[back];
        string->items[back] = tmp;
        front++, back--;
    }
}


String readStreamToString(FILE* file)
{
    String string = {0};
    if (feof(file))
        return string;
    else if (ferror(file))
        clearerr(file);

    long pos;
    assert(fseek(file, 0, SEEK_END) == 0);
    assert((pos = ftell(file)) > 0);
    rewind(file);

    resizeArray(&string, (usize)pos + 1);
    fread(string.items, 1, string.cap, file);
    string.len = (usize)pos;
    return string;
}

String readFileToString(const char* path)
{
    String result = {0};
    FILE* file = fopen(path, "r");
    if (file != NULL) {
        result = readStreamToString(file);
        fclose(file);
    }
    return result;
}

StringView initSvFromString(
    String* str,
    usize   begin,
    usize   end)
{
    StringView ref = {0};
    initSliceFromArray(&ref, str, begin, end);
    return ref;
}

char* cstrArenaPrintf(
    Arena*      arena,
    const char* fmt,
                ...)
{
    va_list va;
    char* buf = NULL;
    int size = 0;

    va_start(va, fmt);
    size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        buf = allocArena(arena, (usize)size + 1);
        va_start(va, fmt);
        vsnprintf(buf, (usize)size + 1, fmt, va);
        va_end(va);
    }

    return buf;
}

char* cstrPrintf(const char* fmt, ...)
{
    va_list va;
    char* buf = NULL;
    int size = 0;

    va_start(va, fmt);
    size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        buf = strictAlloc((usize)size + 1);
        va_start(va, fmt);
        vsnprintf(buf, (usize)size + 1, fmt, va);
        va_end(va);
    }

    return buf;
}

String stringPrintf(const char* fmt, ...)
{
    String str = {0};
    va_list va;
    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    if (size > 0) {
        resizeArray(&str, (usize)size + 1);
        va_start(va, fmt);
        vsnprintf(str.items, str.cap, fmt, va);
        va_end(va);
        str.len += size;
    }

    return str;
}
#endif

#define MISC_FNV_BASIS (0xcbf29ce484222325ULL)
#define MISC_FNV_PRIME (0x100000001b3ULL)

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
That would resulting in double free after the call of freeMap().

K* and V* value from the table will be freed when:
1. Call of deleteFromMap()
2. Call of freeMap()

freeMap() will free all the backing memory of K* and V* thoroughly
until N capacity of table. deleteFromMap() will only free
1 entry of K and V, marking it as tombstone and can be used again
if needed.

*/

typedef struct {
    void* key;
    usize keyLen;
    void* value;
    u64 hash;
} MapEntry;

typedef struct {
    void* key;
    void* value;
    usize keyLen;
    usize pos;
} MapKV;

typedef struct {
    MapEntry* items;
    usize cap;
    usize len;
} Map;

u64 initFNV(const void* ptr, usize size);
void initMap(Map* map);
void putInMap(Map* map, const void* key, usize keyLen, const void* value, usize valueSize);
void* getFromMap(Map* map, const void* key, usize keyLen);
void deleteFromMap(Map* map, const void* key, usize keyLen);
bool iterateMap(Map* map, MapKV* input);
void freeMap(Map* map);

#ifdef MISC_IMPL
#define mapLoadFactor(map) ((f64)(map)->len / (f64)(map)->cap)

void initMap(Map* map)
{
    resizeArray(map, MISC_MAP_MINIMUM);
}

static bool compareKey(
    MapEntry*   dst,
    const void* key,
    usize       keyLen,
    u64         hash)
{
    return dst->keyLen == keyLen &&
           dst->hash   == hash   &&
           memcmp(dst->key, key, keyLen) == 0;
}

static MapEntry* findMapEntry(
    Map*        map,
    const void* key,
    usize       keyLen,
    u64         hash)
{
    usize idx = hash & (map->cap - 1);
    MapEntry* tombstone = NULL;

    while (true) {
        MapEntry* entry = &map->items[idx];
        if (entry->key == NULL) {
            if (entry->value == NULL) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (compareKey(entry, key, keyLen, hash)) {
            return entry;
        }
        idx = (idx + 1) & (map->cap - 1);
    }
}

static void growMap(Map* map, usize into)
{
    Map newer = {0};
    resizeArray(&newer, into);
    newer.len = map->len;

    for (usize i = 0; i < map->cap; i++) {
        MapEntry* entry = &map->items[i];
        if (entry->key == NULL)
            continue;

        MapEntry* dest = findMapEntry(&newer, entry->key, entry->keyLen, entry->hash);
        *dest = *entry;
    }

    freeArray(map);
    *map = newer;
}

void putInMap(
    Map*        map,
    const void* key,
    usize       keyLen,
    const void* value,
    usize       valueSize)
{
    if (map->cap < MISC_MAP_MINIMUM) {
        initMap(map);
    } else if (mapLoadFactor(map) >= MISC_MAP_LOADF) {
        growMap(map, map->cap * 2);
    }

    u64 hash = initFNV(key, keyLen);
    MapEntry* entry = findMapEntry(map, key, keyLen, hash);
    bool isNewKey = entry->key == NULL || (uintptr_t)entry->value == 0xdead;
    if (isNewKey) {
        entry->key = strictAlloc(keyLen);
        entry->value = strictAlloc(valueSize);
        entry->keyLen = keyLen;
        entry->hash = hash;
        memmove(entry->key, key, keyLen);
        map->len++;
    }
    memmove(entry->value, value, valueSize);
}

void* getFromMap(
    Map*        map,
    const void* key,
    usize       keyLen)
{
    MapEntry* entry = findMapEntry(map, key, keyLen, initFNV(key, keyLen));
    if (entry->key != NULL) return entry->value;
    return NULL;
}

void deleteFromMap(
    Map*        map,
    const void* key,
    usize       keyLen)
{
    MapEntry* entry = findMapEntry(map, key, keyLen, initFNV(key, keyLen));
    if (entry->key == NULL) return;

    free(entry->key);
    free(entry->value);
    memset(entry, 0, sizeof *entry);
    entry->value = (void*)0xdead;
    map->len--;
}

void freeMap(Map* map)
{
    for (usize i = 0; i < map->cap; i++) {
        MapEntry entry = map->items[i];
        if (entry.key == NULL || (uintptr_t)entry.value == 0xdead)
            continue;

        free(entry.key);
        free(entry.value);
    }
    freeArray(map);
}

bool iterateMap(Map* map, MapKV* input)
{
    for (; input->pos < map->cap; input->pos++) {
        MapEntry entry = map->items[input->pos];
        if (entry.key != NULL) {
            input->key = entry.key;
            input->value = entry.value;
            input->keyLen = entry.keyLen;
            input->pos++;
            return true;
        }
    }

    input->pos = 0;
    return false;
}

u64 initFNV(const void* ptr, usize size)
{
    const u8* bytes = ptr;
    u64 baseValue = MISC_FNV_BASIS;
    for (u64 i = 0; i < size; i++) {
        baseValue *= MISC_FNV_PRIME;
        baseValue ^= bytes[i];
    }
    return baseValue;
}

#endif

#endif

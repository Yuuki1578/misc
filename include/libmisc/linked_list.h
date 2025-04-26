#ifndef MISC_LINKED_LIST_H
#define MISC_LINKED_LIST_H

#include <libmisc/arena.h>
#include <stddef.h>

#define LL_PENDING -2
#define LL_UNAVAIL -1
#define LL_READY 0

typedef struct CachedLinkedList List;

struct CachedLinkedList {
  List* next;
  void* item;
  size_t size;
  size_t position;
};

extern List list_new(size_t size_type);
extern int list_reserve(Arena* allocator, List* self, size_t count);
extern int list_append(Arena* allocator, List* self, void* item);
extern void list_rewind(List* self);
extern void list_dealloc(Arena* allocator);

#endif

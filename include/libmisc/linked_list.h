#ifndef MISC_LINKED_LIST_H
#define MISC_LINKED_LIST_H

#include "arena.h"
#include <stddef.h>

/* This is a linked list node, contains list from head to toe.
 * The node is a private implementation, sorry.
 * */
typedef struct {
    void* __mem; // You are not supposed to access it directly.
} LinkedListContent;

/* A full blown linked list type, store all the necessary metadata
 * for it to be works.
 * */
typedef struct {
    LinkedListContent list;
    Arena* allocator;
    size_t
        item_size,
        capacity,
        length;
} LinkedList;

LinkedList ll_create(size_t item_size);
bool ll_append(LinkedList* ll);
void ll_free(LinkedList* ll);

#endif

#include "../include/libmisc/linked_list.h"

LinkedList ll_create(size_t item_size)
{
    LinkedList init = {};

    if (item_size < 1)
        return init;

    init.item_size = item_size;
    init.allocator = arena_create(ARENA_PAGE * 1024);
    if (init.allocator == NULL)
        return init;

    void** context = &init.list.__mem;
    *context = arena_alloc(init.allocator, item_size + sizeof(LinkedListContent));
    if (*context == NULL)
        return init;

    return init;
}

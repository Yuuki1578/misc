#include <libmisc/arena.h>
#include <libmisc/linked_list.h>
#include <string.h>

List list_new(size_t size_type) {
  List list = {
      .next = nullptr,
      .item = nullptr,
      .position = 0,
      .size = size_type != 0 ? size_type : sizeof(char),
  };

  return list;
}

int list_reserve(Arena* allocator, List* self, size_t count) {
  List* attach = self;

  if (allocator == nullptr || attach == nullptr) {
    return LL_UNAVAIL;
  }

  for (size_t idx = 0; idx < count; idx++) {
    attach->next = arena_alloc_generic(allocator, sizeof(List));
    attach->item = arena_alloc_generic(allocator, self->size);

    if (attach->next == nullptr || attach->item == nullptr) {
      return LL_UNAVAIL;
    }

    attach->next->position = attach->position + 1;
    attach = attach->next;
  }

  return LL_READY;
}

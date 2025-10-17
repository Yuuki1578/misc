#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"

int main(void) {
    ARENA_INIT();

    List list = {
        .item_size = 64,
    };

    for (int i = 1; i <= 1 << 12; i++) {
        char buf[64] = {0};
        sprintf(buf, "iteration: %d", i);
        list_append(&list, buf);
    }

    for (RawList *each = list.tail; each != NULL; each = each->prev) {
        char *buf = each->item;
        printf("%s\n", buf);
    }

    ARENA_DEINIT();
}

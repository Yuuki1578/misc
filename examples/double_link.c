#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"

int main(void)
{
    ARENA_INIT();

    Double_Link dlink = {
        .item_size = 64,
    };

    for (int i = 1; i <= 1 << 12; i++) {
        char buf[64] = {};
        sprintf(buf, "Iteration: %d", i);
        dl_append(&dlink, buf);
    }

    for (Raw_Double_Link* head = dlink.head; head != NULL; head = head->next) {
        printf("%s\n", (char*)head->item);
    }

    misc_free();
}

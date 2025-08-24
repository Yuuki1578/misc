#include "../misc.h"

int main(void)
{
    Double_Link dlink = {
        .item_size = 64,
    };

    for (int i = 0; i < 1000; i++) {
        char buf[64] = {};
        sprintf(buf, "Iteration: %d", i);
        dl_append(&dlink, buf);
    }

    for (Raw_Double_Link* head = dlink.head; head != NULL; head = head->next) {
        printf("%s\n", (char*)head->item);
    }

    dl_free(&dlink);
}

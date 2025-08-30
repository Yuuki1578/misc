#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"
#include <functional>

template <class T>
struct Link {
    Double_Link dlink;

    Link<T>(void)
        : dlink({ .item_size = sizeof(T) })
    {
    }

    void append(T item)
    {
        dl_append(&dlink, &item);
    }

    void prepend(T item)
    {
        dl_prepend(&dlink, &item);
    }

    void for_each(std::function<void(T&)> fn)
    {
        auto head = dlink.head;
        while (head != NULL) {
            auto* item = (T*)head->item;
            fn(*item);
            head = head->next;
        }
    }
};

int main(void)
{
    ARENA_INIT();

    auto link = Link<int>();
    for (int i = 1; i <= 1 << 12; i++) {
        link.append(i * 10);
    }

    link.for_each([](auto& item) {
        item /= 10;
        printf("item: %d\n", item);
    });

    misc_free();
}

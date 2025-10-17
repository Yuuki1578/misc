#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"
#include <functional>

template <class T>
struct CppList {
    List list;

    CppList<T>(void) : list({nullptr, nullptr, sizeof(T), 0}) {}

    void append(T item)
    {
        list_append(&list, &item);
    }

    void prepend(T item)
    {
        list_prepend(&list, &item);
    }

    void for_each(std::function<void(T &)> callback)
    {
        auto *head = list.head;
        while (head != NULL) {
            auto *item = static_cast<T *>(head->item);
            callback(*item);
            head = head->next;
        }
    }
};

int main(void)
{
    ARENA_INIT();

    auto list = CppList<int>();
    for (int i = 1; i <= 1 << 12; i++) {
        list.append(i * 10);
    }

    list.for_each([](auto &item) {
        item /= 10;
        printf("item: %d\n", item);
    });

    ARENA_DEINIT();
}

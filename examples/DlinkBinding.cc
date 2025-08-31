#define MISC_USE_GLOBAL_ALLOCATOR
#include "../Misc.h"
#include <functional>

template <class T> struct Link {
  DoubleLink Dlink;

  Link<T>(void) : Dlink({nullptr, nullptr, 0, 0}) {}
  void append(T Item) { doubleLinkAppend(&Dlink, &Item); }
  void prepend(T Item) { doubleLinkPrepend(&Dlink, &Item); }
  void forEach(std::function<void(T &)> CallbackFn) {
    auto HeadLink = Dlink.Head;
    while (HeadLink != NULL) {
      auto *Item = (T *)HeadLink->Item;
      CallbackFn(*Item);
      HeadLink = HeadLink->Next;
    }
  }
};

int main(void) {
  ARENA_INIT();

  auto NewLink = Link<int>();
  for (int I = 1; I <= 1 << 12; I++) {
    NewLink.append(I * 10);
  }

  NewLink.forEach([](auto &Item) {
    Item /= 10;
    printf("item: %d\n", Item);
  });

  ARENA_DROP();
}

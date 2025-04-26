// Program example

#include <libmisc/arena.h>
#include <libmisc/linked_list.h>
#include <stdio.h>
#include <string.h>

typedef struct link* link;
struct link {
  link next;
  int data;
};

void test_ll(void) {
  List list = list_new(sizeof(int));
  Arena alloc = {0};

  arena_new(&alloc, ARENA_STEP_DFL);
  list_reserve(&alloc, &list, 10);
}

void test_independent_arena(void) {
  // Non-global arena with default size
  Arena arena;
  arena_new(&arena, ARENA_STEP_DFL);

  char* some_string = arena_alloc_generic(&arena, 1 << 8);
  strcpy(some_string, "C:");
  strcat(some_string, __FILE__);

  some_string = arena_realloc_generic(&arena, some_string, 1 << 8, 1 << 10);

  // big chunk
  void* trash = arena_alloc(PAGE_SIZE);
  void* other_trash =
      arena_realloc_generic(&arena, trash, PAGE_SIZE, PAGE_SIZE * 2);
  memset(other_trash, 255, PAGE_SIZE * 2);

  arena_snapshot(&arena);
  arena_dealloc_generic(&arena);
  putchar('\n');
  fflush(stdout);
}

void test_linked_list(void) {
  link linked = arena_alloc(sizeof(struct link));
  link represent = linked;

  if (!represent) {
    return;
  }

  for (int i = 1; i <= 10; i++) {
    represent->data = i * 100;
    represent->next = arena_alloc(sizeof(struct link));
    printf("Link %d: %d\n", i, represent->data);

    if (!represent->next) {
      break;
    }

    represent = represent->next;
  }

  putchar('\n');
  fflush(stdout);
}

void test_realloc(void) {
  // first allocation
  int* array = arena_alloc(sizeof(int) * 3);

  if (!array) {
    return;
  }

  array[0] = 1 << 8;
  array[1] = 1 << 9;
  array[2] = 1 << 10;

  for (int i = 0; i < 3; i++) {
    printf("%d\n", array[i]);
  }

  putchar('\n');

  // realloc!
  array = arena_realloc(array, sizeof(int) * 3, sizeof(int) * 5);
  array[3] = 1 << 11;
  array[4] = 1 << 12;

  for (int i = 0; i < 5; i++) {
    printf("%d\n", array[i]);
  }

  putchar('\n');
  fflush(stdout);

  // Here the function scope is ended
  // all the variable is pop-out from the stack.
  // But here is the catch:
  // the data of @array is still present!
  // it still live until the heat death of the universe!
  // (or when you call @arena_dealloc)
}

void test_heavy_string(void) {
  // allocate 64 bytes
  char* s = arena_alloc(64);
  arena_snapshot_global();
  putchar('\n');

  // allocate 1024 bytes
  s = arena_alloc(1024);
  arena_snapshot_global();
  putchar('\n');

  // allocate 12 bytes
  s = arena_alloc(12);
  strcpy(s, "Booo!");
  arena_snapshot_global();
  putchar('\n');

  // reallocate 72 bytes from 12 bytes
  s = arena_realloc(s, 12, 72);

  // content still present
  puts(s);
  arena_snapshot_global();
  putchar('\n');
  fflush(stdout);
}

int main(void) {
  // change default size at runtime
  ARENA_STEP_DFL = PAGE_SIZE * 2;

  // initialize global arena with @ARENA_STEP_DFL
  arena_global_initializer();

  test_ll();
  // test_independent_arena();
  // test_linked_list();
  // test_realloc();
  // test_heavy_string();

  // deallocate global arena
  arena_dealloc();
}

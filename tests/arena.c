#include <libmisc/arena.h>
#include <stdio.h>
#include <threads.h>

Arena *arena = &(Arena){};

/*
 * This string will be "0123456789"
 *
 */
static char *str = nullptr;
static size_t offset = 0, count = 0;

mtx_t mutex;

static int change_string(void *args) {
  mtx_lock(&mutex);
  (void)args;

  char ascii_digit = (count++) + '0';
  memcpy(str + (offset++), &ascii_digit, 1);

  mtx_unlock(&mutex);
  return 0;
}

int main(void) {
  if (arena_init(arena, PAGE_SIZE, true) != ARENA_READY)
    return 1;

  mtx_init(&mutex, mtx_plain);
  if ((str = arena_alloc(arena, 32)) == nullptr)
    goto deinit;

  thrd_t jobs[10];
  for (int i = 0; i < 10; i++)
    thrd_create(&jobs[i], change_string, nullptr);

  thrd_yield();
  for (int i = 0; i < 10; i++)
    thrd_join(jobs[i], nullptr);

  printf("%s\n", str);

deinit:
  mtx_destroy(&mutex);
  arena_dealloc(arena);
}

#include "../misc.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

int thread_routine(void* args)
{
    int* data = args;
    struct timespec timer = { .tv_sec = 1 };

    // lock
    refcount_strong(&args);
    *data += 1;

    // lock
    refcount_weak(&args);
    printf("thread %d is done!\n"
           "lifetime: %zu\n",

        // lock
        *data, refcount_lifetime(&args));

    thrd_sleep(&timer, NULL);
    return 0;
}

int main(void)
{
    Vector handler_list = vector_with(10, sizeof(thrd_t));
    int* data = refcount_alloc(sizeof(int));
    void* holder = data;

    for (int i = 0; i < 10; i++) {
        thrd_t handle;
        thrd_create(&handle, thread_routine, data);
        vector_push(&handler_list, &handle);
    }

    for (size_t i = 0; i < handler_list.length; i++) {
        thrd_t* handle = vector_at(&handler_list, i);
        thrd_join(*handle, NULL);
    }

    printf("now: %d\n", *data);
    vector_free(&handler_list);
    refcount_weak(&holder);

    printf("lifetime: %zu\n", refcount_lifetime(&holder));
    printf("address: %p\n", holder);
}

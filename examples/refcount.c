#include "../misc.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>


int thread_routine(void *args)
{
    int *data = args;
    struct timespec timer = {.tv_sec = 1};

    // lock
    refcount_upgrade(&args);
    *data += 1;

    // lock
    refcount_degrade(&args);
    printf("thread %d is done!\n"
           "lifetime: %zu\n",

           // lock
           *data, refcount_lifetime(&args));

    thrd_sleep(&timer, NULL);
    return 0;
}

int main(void)
{
    Vector handlers = vector_create_with(10, sizeof(thrd_t));
    int *data = refcount_alloc(sizeof(int));
    void *holder = data;

    for (int i = 0; i < 10; i++) {
        thrd_t handle;
        thrd_create(&handle, thread_routine, data);
        vector_push(&handlers, &handle);
    }

    for (size_t i = 0; i < handlers.length; i++) {
        thrd_t *handle = vector_get(&handlers, i);
        thrd_join(*handle, NULL);
    }

    printf("now: %d\n", *data);
    vector_free(&handlers);
    refcount_degrade(&holder);

    printf("lifetime: %zu\n", refcount_lifetime(&holder));
    printf("address: %p\n", holder);
}

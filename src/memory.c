#include <ced/memory.h>
#include <stdint.h>
#include <stdlib.h>

layout_t layout_new(uint16_t t_size, size_t default_len)
{
    if (t_size == 0)
        t_size = sizeof(char);

    return (layout_t) {
        .t_size = t_size,
        .cap    = t_size * default_len,
        .len    = default_len,
        .status = NULL_PTR,
    };
}

void layout_add(layout_t *layout, size_t count)
{
    if (layout == nullptr || count == 0)
        return;

    layout->cap += ((layout->t_size != 0 ? layout->t_size : sizeof(char)) * count);
    layout->len += count;
}

void layout_min(layout_t *layout, size_t count)
{
    if (layout == nullptr || count == 0)
        return;

    if (layout->len < count)
        return;

    layout->cap -= ((layout->t_size != 0 ? layout->t_size : sizeof(char)) * count);
    layout->len -= count;
}

void *layout_alloc(layout_t *layout)
{
    void *dump;

    if (layout == nullptr)
        return calloc(CED_ALLOC_UNSPEC, sizeof(char));

    if (layout->t_size == 0)
        layout->t_size = sizeof(char);

    // cannot allocate more than default limit (3GiB)
    if (layout->cap == 0 || layout->cap > CED_ALLOC_LIMIT) {
        dump = calloc(CED_ALLOC_UNSPEC, layout->t_size);

        if (dump == nullptr)
            layout->status = NULL_PTR;

        layout->status = UNIQUE_PTR;
        return dump;
    }

    dump = calloc(layout->len, layout->t_size);

    if (dump == nullptr)
        layout->status = NULL_PTR;
    else
        layout->status = NON_NULL;

    return dump;
}

void *layout_realloc(layout_t *layout, void *dst)
{
    void *dump;

    if (layout == nullptr || dst == nullptr)
        return layout_alloc(nullptr);

    dump = realloc(dst, layout->cap);

    if (dump == nullptr)
        layout->status = NULL_PTR;

    return dump;
}

void layout_dealloc(layout_t *layout, void *dst)
{
    if (layout == nullptr || layout->status != NON_NULL) {
        if (layout->status == NULL_PTR)
            return;
    }

    free(dst);

    // invalidate the pointer
    dst = nullptr;
    layout->cap = 0;
    layout->len = 0;
    layout->status = NULL_PTR;
}

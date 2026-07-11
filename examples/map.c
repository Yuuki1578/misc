#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    hashmap_t map = { 0 };
    const int K = 1024 << 2;
    hashkey_t key = {
        .key = (void*) &K,
        .len = sizeof K,
    };

    bool ok = hashmap_put(
        &map,
        key,
        &key,
        sizeof key);
    assert(ok);

    hashentry_t *entry = hashmap_get_entry(
        &map,
        key);
    if (entry != NULL) {
        hashkey_t *keyptr = entry->value;
        printf(
            "key size: %zu, hash: %zu\n",
            keyptr->len,
            keyptr->hash);
    }

    return 0;
}

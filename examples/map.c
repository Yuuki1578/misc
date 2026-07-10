#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    Map *map = CreateMap();
    assert(map != NULL);

    int k, v, *val;
    MapKey key;

    k = 1;
    v = k * 100;
    key.key = (void *)&k;
    key.len = sizeof k;
    assert(PutIntoMap(map, key, &v, sizeof v));

    k = 2;
    v = k * 100;
    key.key = (void *)&k;
    key.len = sizeof k;
    assert(PutIntoMap(map, key, &v, sizeof v));

    k = 3;
    v = k * 100;
    key.key = (void *)&k;
    key.len = sizeof k;
    assert(PutIntoMap(map, key, &v, sizeof v));

    k = 1;
    key.key = (void *)&k;
    key.len = sizeof k;
    val = GetFromMap(map, key);
    assert(val);
    printf("%d\n", *val);

    k = 2;
    key.key = (void *)&k;
    key.len = sizeof k;
    val = GetFromMap(map, key);
    assert(val);
    printf("%d\n", *val);

    k = 3;
    key.key = (void *)&k;
    key.len = sizeof k;
    assert(DeleteFromMap(map, key));

    k = 3;
    key.key = (void *)&k;
    key.len = sizeof k;
    val = GetFromMap(map, key);
    assert(val == NULL);

    FreeMap(map);
    map = NULL;
}

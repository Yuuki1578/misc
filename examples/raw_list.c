#if defined(__cplusplus) || __STDC_VERSION__ < 201700L
int main(void)
{
    return 0;
}
#endif
#include "../misc.h"


int main(void)
{
    RawList *list = r_list_create(ADDRESS_OF(0), sizeof(int)), *save;

    if (list == NULL)
        return 1;

    save = list;
    r_list_prepend(&list, ADDRESS_OF(-1), sizeof(int));
    r_list_prepend(&list, ADDRESS_OF(-2), sizeof(int));
    r_list_prepend(&list, ADDRESS_OF(-3), sizeof(int));

    list = save;
    r_list_append(&list, ADDRESS_OF(1), sizeof(int));
    r_list_append(&list, ADDRESS_OF(2), sizeof(int));
    r_list_append(&list, ADDRESS_OF(3), sizeof(int));

    for (typeof(list) current = r_list_rewind(&list); current != NULL; current = current->next) {
        int *item = current->item;
        printf("%d\n", *item);
    }

    r_list_free(list);
}

#include "../misc.h"

int main(void)
{
    Raw_Double_Link *rdl = rdl_new(address_of(0), sizeof(int)), *save;

    if (rdl == NULL)
        return 1;

    save = rdl;
    rdl_prepend(&rdl, address_of(-1), sizeof(int));
    rdl_prepend(&rdl, address_of(-2), sizeof(int));
    rdl_prepend(&rdl, address_of(-3), sizeof(int));

    rdl = save;
    rdl_append(&rdl, address_of(1), sizeof(int));
    rdl_append(&rdl, address_of(2), sizeof(int));
    rdl_append(&rdl, address_of(3), sizeof(int));

    for (typeof(rdl) current = rdl_rewind(&rdl); current != NULL; current = current->next) {
        int* item = current->item;
        printf("%d\n", *item);
    }

    rdl_free(rdl);
}

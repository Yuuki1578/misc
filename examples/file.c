#include "../misc.h"
#include <stdlib.h>

int main(void)
{
    char* file_content = file_readall(__FILE__);
    if (file_content != NULL) {
        printf("%s", file_content);
        free(file_content);
    }
}

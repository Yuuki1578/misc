#include "../misc.h"
#include <stdlib.h>

int main(void)
{
    char* file_content = file_read_all(__FILE__);
    if (file_content != NULL) {
        printf("%s", file_content);
        free(file_content);
    }
}

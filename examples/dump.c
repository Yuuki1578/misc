#define MISC_IMPL
#include "../misc.h"

int main(void)
{
    u8 buf[sizeof(void*)];
    void* foo = (void*)0xfafafa;
    memmove(buf, &foo, 8);
}

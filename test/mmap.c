#include "libmisc/ipc/mmap.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    // Initiate mapping.
    // The mapping is in sync between process because it
    // share the same virtual address.
    SetMapping *map = SetMappingNew(512);
    if (map == NULL) {
        return 1;
    }

    switch (fork()) {
    case -1: // fork() fail
        SetMappingClose(map);
        return 2;

    case 0: // Child process with a different address space
        char buf[] = "Hello, world!\n";
        SetMappingWrite(map, buf, sizeof(buf) - 1);
        SetMappingClose(map);
        return 0;

    default:        // Parent process
        wait(NULL); // Wait for child process to complete
        char msg[32] = {0};

        SetMappingRewind(map);
        SetMappingRead(map, msg, sizeof msg);
        printf("%s", msg);
        SetMappingClose(map);
    }

    return 0;
}

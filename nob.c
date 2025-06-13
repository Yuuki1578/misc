#include <asm/fcntl.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#ifdef __clang__
#define MISC_CC "clang"
#define MISC_AR "llvm-ar"
#else
#define MISC_CC "gcc"
#define MISC_AR "ar"
#endif

#define BUILD_DIR "build/"

#define MISC_CC_FLAGS                                                                              \
    "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c23", "-O3", "-Wno-pointer-arith",           \
        "-ffast-math", "-fomit-frame-pointer", "-funroll-loops", "-march=native", "-mtune=native", \
        "-flto"

static bool CheckBuildDirectory(void)
{
    int dirfd = open(BUILD_DIR, O_DIRECTORY);
    if (dirfd != -1) {
        close(dirfd);
        return true;
    }

    close(dirfd);
    if (mkdir(BUILD_DIR, 0777) == -1) {
        return false;
    }

    return true;
}

static bool InsertFile(Nob_Cmd *cmd, ...)
{
    va_list va;
    char *file_name;

    if (!CheckBuildDirectory())
        return false;

    va_start(va, cmd);

    while ((file_name = va_arg(va, char *)) != NULL) {
        char resolve_path[512] = {0};
        char *occur            = strstr(file_name, "/") + 1;

        strcat(resolve_path, BUILD_DIR);
        strcat(resolve_path, occur);
        resolve_path[strcspn(resolve_path, ".") + 1] = 'o';
        nob_cmd_append(cmd, MISC_CC, MISC_CC_FLAGS, file_name, "-c", "-o", resolve_path);

        if (!nob_cmd_run_sync_and_reset(cmd)) {
            return false;
        }
    }

    return true;
}

static bool CreateLibrary(Nob_Cmd *cmd)
{
    if (!CheckBuildDirectory())
        return false;

    if (!InsertFile(cmd, "src/arena.c", "src/nonblock.c", "src/tcp.c"))
        return false;

    nob_cmd_append(cmd, MISC_CC, MISC_CC_FLAGS, "build/arena.o", "build/nonblock.o", "build/tcp.o",
                   "-fPIC", "-shared", "-o", "build/libmisc.so");
    NOB_ASSERT(nob_cmd_run_sync_and_reset(cmd));

    return true;
}

int main(int argc, char **argv)
{
    Nob_Cmd cmd = {0};
    NOB_GO_REBUILD_URSELF(argc, argv);
    // MiscInsertFile(&cmd, "src/arena.c", "src/nonblock.c", "src/tcp.c", NULL);
    CreateLibrary(&cmd);

    nob_cmd_free(cmd);
    return 0;
}

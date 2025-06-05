#define NOB_IMPLEMENTATION
#if defined(_WIN32) || defined(_WIN64)
#error Use cmake build system instead
#endif

#include "nob.h"
#include <fcntl.h>

#ifdef __clang__
#define CC "clang"
#define AR "llvm-ar"
#else
#define CC "gcc"
#define AR "ar"
#endif

#define SHAREDLIB "build/libmisc.so"

#define CFILES              \
    "./src/arena.c",        \
        "./src/nonblock.c", \
        "./src/ipc.c"

#define CFLAGS                  \
    "-Wall",                    \
        "-Werror",              \
        "-Wextra",              \
        "-Wno-pointer-arith",   \
        "-pedantic",            \
        "-std=c23",             \
        "-O3",                  \
        "-flto",                \
        "-ffast-math",          \
        "-funroll-loops",       \
        "-fomit-frame-pointer", \
        "-march=native",        \
        "-mtune=native",        \
        "-Iinclude",            \
        "-fPIC",                \
        "-shared",              \
        "-s"

static int check_build_dir(void)
{

    int builddir = open("./build", O_DIRECTORY);
    if (builddir == -1) {
        if (mkdir("./build", 0777) != 0)
            return -1;
    } else {
        close(builddir);
    }

    return 0;
}

int main(int argc, char** argv)
{
    Nob_Cmd cmd = { 0 };
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (check_build_dir() != 0)
        goto cleanup;

    nob_cmd_append(&cmd, CC, CFILES, CFLAGS, "-o", SHAREDLIB);

    if (!nob_cmd_run_sync_and_reset(&cmd))
        return 1;

cleanup:
    nob_cmd_free(cmd);
    return 0;
}

#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#ifdef _MSVC_VER
#    error MSVC is not supported
#endif

#ifdef __clang__
#    define CC "clang"
#    define AR "llvm-ar"
#elif defined(__GNUC__) && !defined(__clang__)
#    define CC "gcc"
#    define AR "ar"
#endif

#define CFLAGS                                                                 \
    "-Wall", "-Werror", "-Wextra", "-std=c11", "-pedantic", "-ffast-math",     \
        "-fomit-frame-pointer", "-funroll-loops", "-O2", "-march=native",      \
        "-mtune=native"

#define ARFLAGS    "rcs"
#define STATIC_LIB "build/libmisc.a"

static char *srcs[] = {
    "src/arena.c",
    "src/string.c",
    "src/vector.c",
    "src/reference_counting.c",
};

static char *objs[] = {
    "build/arena.o",
    "build/string.o",
    "build/vector.o",
    "build/reference_counting.o",
};

static char *examples[] = {
    "examples/arena.c",
    "examples/string.c",
    "examples/vector.c",
    "examples/reference_counting.c",
};

static char *exe_amples[] = {
    "build/examples/arena",
    "build/examples/string",
    "build/examples/vector",
    "build/examples/reference_counting",
};

static size_t file_total = sizeof srcs / sizeof *srcs;

void compile_source_files(Nob_Cmd *cmd)
{
    for (size_t i = 0; i < file_total; i++) {
        nob_cmd_append(cmd, CC, CFLAGS, srcs[i], "-c", "-o", objs[i]);
        nob_cmd_run_sync_and_reset(cmd);
    }
}

void archive_object_files(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, AR, ARFLAGS, STATIC_LIB, objs[0], objs[1], objs[2],
                   objs[3]);
    nob_cmd_run_sync_and_reset(cmd);
}

void create_examples(Nob_Cmd *cmd)
{
    for (size_t i = 0; i < file_total; i++) {
        nob_cmd_append(cmd, CC, CFLAGS, "-fsanitize=address",
                       "-Wno-overlength-strings", "-Lbuild", "-lmisc",
                       examples[i], "-o", exe_amples[i]);
        nob_cmd_run_sync_and_reset(cmd);
    }
}

int main(int argc, char **argv)
{
    Nob_Cmd cmd = {0};

    NOB_GO_REBUILD_URSELF(argc, argv);
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    compile_source_files(&cmd);
    archive_object_files(&cmd);
    create_examples(&cmd);

    nob_cmd_free(cmd);
    return 0;
}

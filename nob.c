/*

======= Copyright (c) 2024 Alexey Kutepov =======
         Licensed under the MIT License

=====================================================================================
DISCLAIMER:
This is a third party build system, all right reserved to the author of this library.
=====================================================================================

*/

#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#ifdef __clang__
#define CC "clang"
#define AR "llvm-ar"
#define FMT "clang-format"
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
#define AR "ar"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS                  \
    "-Wall",                    \
        "-Werror",              \
        "-Wextra",              \
        "-std=c23",             \
        "-pedantic",            \
        "-ffast-math",          \
        "-fomit-frame-pointer", \
        "-funroll-loops",       \
        "-march=native",        \
        "-mtune=native"

#define ARFLAGS "rcs"

/* ===== CORE FILES ===== */
static void core_compile_only(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output);
static void core_compile_only_all(Nob_Cmd* cmd, Nob_Procs* procs);
static void core_create_archive(Nob_Cmd* cmd, Nob_Procs* procs);
/* ===== CORE FILES ===== */

/* ===== EXAMPLES ===== */
static void examples_compile(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output);
static void examples_compile_all(Nob_Cmd* cmd, Nob_Procs* procs);
/* ===== EXAMPLES ===== */

/* ===== MISC ===== */
static void source_format(Nob_Cmd* cmd);
/* ===== MISC ===== */

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = { 0 };
    Nob_Procs procs = { 0 };

    core_compile_only_all(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    core_create_archive(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    examples_compile_all(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    if (argc > 1) {
        if (strcmp(argv[1], "--format") == 0) {
#ifdef __clang__
            source_format(&cmd);
#else
            (void)0;
#endif
        } else if (strcmp(argv[1], "--clean") == 0) {
            DIR* dir = opendir("build");
            if (dir != NULL) {
                closedir(dir);
                remove("build");
            }

        } else {
            nob_cmd_append(&cmd, nob_temp_sprintf("./build/examples/%s", argv[1]));
            if (!nob_cmd_run_sync_and_reset(&cmd))
                return 1;
        }
    }

    return 0;
}

static void core_compile_only(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output)
{
    nob_cc(cmd);
    nob_cmd_append(cmd, CFLAGS, "-c");
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void core_compile_only_all(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build");

    core_compile_only(cmd, procs, "src/arena.c", "build/arena.o");
    core_compile_only(cmd, procs, "src/vector.c", "build/vector.o");
    core_compile_only(cmd, procs, "src/string.c", "build/string.o");
    core_compile_only(cmd, procs, "src/refcount.c", "build/refcount.o");
    core_compile_only(cmd, procs, "src/file.c", "build/file.o");
}

static void core_create_archive(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build");

    nob_cmd_append(cmd,
        AR,
        ARFLAGS,
        "build/libmisc.a",

        /* ===== OBJECT FILES ===== */
        "build/arena.o",
        "build/vector.o",
        "build/string.o",
        "build/refcount.o",
        "build/file.o",
        /* ===== OBJECT FILES ===== */
    );

    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void examples_compile(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output)
{
    nob_cc(cmd);
#ifdef __clang__
    /* It has a builtin sanitizer */
    nob_cmd_append(cmd, CFLAGS, "-Wno-overlength-strings", "-fsanitize=address");
#else
    /* Can your gcc do that? */
    nob_cmd_append(cmd, CFLAGS, "-Wno-overlength-strings");
#endif
    nob_cc_inputs(cmd, input, "build/libmisc.a");
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void examples_compile_all(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build/examples");

    examples_compile(cmd, procs, "examples/arena.c", "build/examples/arena");
    examples_compile(cmd, procs, "examples/vector.c", "build/examples/vector");
    examples_compile(cmd, procs, "examples/string.c", "build/examples/string");
    examples_compile(cmd, procs, "examples/refcount.c", "build/examples/refcount");
    examples_compile(cmd, procs, "examples/list.c", "build/examples/list");
    examples_compile(cmd, procs, "examples/file_reading.c", "build/examples/file_reading");
    examples_compile(cmd, procs, "examples/numeric.c", "build/examples/numeric");
    examples_compile(cmd, procs, "examples/file.c", "build/examples/file");
}

#ifdef __clang__

#define format_file(file)                     \
    do {                                      \
        nob_cmd_append(cmd, FMT, file, "-i"); \
        nob_cmd_run_async_and_reset(cmd);     \
    } while (0);

static void source_format(Nob_Cmd* cmd)
{
    format_file("src/arena.c");
    format_file("src/refcount.c");
    format_file("src/string.c");
    format_file("src/vector.c");
    format_file("src/file.c");

    format_file("include/libmisc/arena.h");
    format_file("include/libmisc/list.h");
    format_file("include/libmisc/refcount.h");
    format_file("include/libmisc/string.h");
    format_file("include/libmisc/vector.h");
    format_file("include/libmisc/file.h");

    format_file("examples/arena.c");
    format_file("examples/refcount.c");
    format_file("examples/string.c");
    format_file("examples/vector.c");
    format_file("examples/file_reading.c");
    format_file("examples/numeric.c");
    format_file("examples/file.c");

    format_file("nob.c");
}

#endif

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
#define FMT "clang-format"
#define ADDRESS_SANITIZER
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
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
        } else {
            nob_cmd_append(&cmd, nob_temp_sprintf("./build/examples/%s", argv[1]));
            if (!nob_cmd_run_sync_and_reset(&cmd))
                return 1;
        }
    }

    return 0;
}

static void examples_compile(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output)
{
    nob_cc(cmd);
#if defined(__clang__) && defined(ADDRESS_SANITIZER)
    /* It has a builtin sanitizer */
    nob_cmd_append(cmd, CFLAGS, "-Wno-overlength-strings", "-fsanitize=address");
#else
    /* Can your gcc do that? */
    nob_cmd_append(cmd, CFLAGS, "-Wno-overlength-strings");
#endif
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void examples_compile_all(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    examples_compile(cmd, procs, "examples/arena.c", "build/examples/arena");
    examples_compile(cmd, procs, "examples/vector.c", "build/examples/vector");
    examples_compile(cmd, procs, "examples/string.c", "build/examples/string");
    examples_compile(cmd, procs, "examples/refcount.c", "build/examples/refcount");
    examples_compile(cmd, procs, "examples/list.c", "build/examples/list");
    examples_compile(cmd, procs, "examples/file_reading.c", "build/examples/file_reading");
    examples_compile(cmd, procs, "examples/numeric.c", "build/examples/numeric");
    examples_compile(cmd, procs, "examples/file.c", "build/examples/file");
    examples_compile(cmd, procs, "examples/linked_list.c", "build/examples/linked_list");
}

#ifdef __clang__

#define format_file(file)                     \
    do {                                      \
        nob_cmd_append(cmd, FMT, file, "-i"); \
        nob_cmd_run_async_and_reset(cmd);     \
    } while (0);

static void source_format(Nob_Cmd* cmd)
{
    format_file("examples/arena.c");
    format_file("examples/refcount.c");
    format_file("examples/string.c");
    format_file("examples/vector.c");
    format_file("examples/file_reading.c");
    format_file("examples/numeric.c");
    format_file("examples/file.c");
    format_file("examples/linked_list.c");

    format_file("nob.c");
}

#endif

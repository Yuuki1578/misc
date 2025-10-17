/*

======= Copyright (c) 2024 Alexey Kutepov =======
         Licensed under the MIT License

=====================================================================================
DISCLAIMER:
This is a third party build system, all right reserved to the author of this
library.
=====================================================================================

*/

#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#ifdef __clang__
#define CC "clang"
#define CXX "clang++"
#define FMT "clang-format"
#define ADDRESS_SANITIZER
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
#define CXX "g++"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS                                                             \
    "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c23", "-ffast-math", \
    "-fomit-frame-pointer", "-funroll-loops", "-march=native",             \
    "-mtune=native", "-Wno-unused-function", "-O0", "-ggdb"

/* ===== EXAMPLES ===== */
void example_compile(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
void example_compile_cpp(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
void example_compile_all(Nob_Cmd *cmd, Nob_Procs *procs);
void example_compile_all_cpp(Nob_Cmd *cmd, Nob_Procs *procs);
/* ===== EXAMPLES ===== */

/* ===== MISC ===== */
void source_format(Nob_Cmd *cmd);
/* ===== MISC ===== */

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    Nob_Procs procs = {0};

    example_compile_all(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    example_compile_all_cpp(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    return 0;
}

void example_compile(Nob_Cmd *cmd,
                     Nob_Procs *procs,
                     char *input,
                     char *output)
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

void example_compile_cpp(Nob_Cmd *cmd,
                         Nob_Procs *procs,
                         char *input,
                         char *output)
{
    nob_cmd_append(cmd, CXX, input, "-o", output,
#if defined(__clang__) && defined(ADDRESS_SANITIZER)
    "-fsanitize=address",
#endif
    "-O0", "-ggdb");

    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

void example_compile_all(Nob_Cmd *cmd, Nob_Procs *procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    example_compile(cmd, procs, "examples/list.c", "build/examples/list");
    example_compile(cmd, procs, "examples/file.c", "build/examples/file");
    example_compile(cmd, procs, "examples/file_reading.c", "build/examples/file_reading");
    example_compile(cmd, procs, "examples/hexdump.c", "build/examples/hexdump");
    example_compile(cmd, procs, "examples/forward_list.c", "build/examples/forward_list");
    example_compile(cmd, procs, "examples/numeric_str.c", "build/examples/numeric_str");
    example_compile(cmd, procs, "examples/raw_list.c", "build/examples/raw_list");
    example_compile(cmd, procs, "examples/refcount.c", "build/examples/refcount");
    example_compile(cmd, procs, "examples/string.c", "build/examples/string");
    example_compile(cmd, procs, "examples/vector.c", "build/examples/vector");
    example_compile(cmd, procs, "examples/arena.c", "build/examples/arena");

}

void example_compile_all_cpp(Nob_Cmd *cmd, Nob_Procs *procs)
{
    example_compile_cpp(cmd, procs, "examples/list_binding.cpp", "build/examples/list_binding");
}

#ifdef __clang__

#define FORMAT_FILE(file)                     \
    do {                                      \
        nob_cmd_append(cmd, FMT, file, "-i"); \
        nob_cmd_run_async_and_reset(cmd);     \
    } while (0);

void source_format(Nob_Cmd *cmd)
{
    FORMAT_FILE("examples/list_binding.cpp");
    FORMAT_FILE("examples/list.c");
    FORMAT_FILE("examples/file.c");
    FORMAT_FILE("examples/file_reading.c");
    FORMAT_FILE("examples/hexdump.c");
    FORMAT_FILE("examples/forward_list.c");
    FORMAT_FILE("examples/numeric_str.c");
    FORMAT_FILE("examples/raw_list.c");
    FORMAT_FILE("examples/refcount.c");
    FORMAT_FILE("examples/string.c");
    FORMAT_FILE("examples/vector.c");
    FORMAT_FILE("examples/arena.c");

    FORMAT_FILE("misc.h");
    FORMAT_FILE("nob.c");
}

#endif

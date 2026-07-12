/*

======= Copyright (c) 2024 Alexey Kutepov =======
         Licensed under the MIT License

=====================================================================================
DISCLAIMER:
This is a third party build system, all rights reserved to the author of this
library.
=====================================================================================

*/

#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#ifdef __clang__
#define CC "clang"
#define CXX "clang++"
// clang-tools-extra
#define FMT "clang-format"
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
#define CXX "g++"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c99", "-ggdb"

void example_cc(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output);
void example_cc_all(Nob_Cmd* cmd, Nob_Procs* procs);
void source_format(Nob_Cmd* cmd);

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = { 0 };
    Nob_Procs procs = { 0 };

    example_cc_all(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs)) {
        return 1;
    }

#ifdef __clang__
    // source_format(&cmd);
#endif

    return 0;
}

void example_cc(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output)
{
    nob_cmd_append(cmd, CC, CFLAGS);
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

void example_cc_all(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    example_cc(cmd, procs, "examples/array.c", "build/examples/array");
    example_cc(cmd, procs, "examples/arena.c", "build/examples/arena");
    example_cc(cmd, procs, "examples/map.c", "build/examples/map");
}

#ifdef __clang__

#define FORMAT_FILE(file)                     \
    do {                                      \
        nob_cmd_append(cmd, FMT, file, "-i"); \
        nob_cmd_run_async_and_reset(cmd);     \
    } while (0);

void source_format(Nob_Cmd* cmd)
{
    FORMAT_FILE("misc.h");
    FORMAT_FILE("nob.c");
    FORMAT_FILE("examples/array.c");
    FORMAT_FILE("examples/arena.c");
    FORMAT_FILE("examples/map.c");
    FORMAT_FILE("examples/map.cpp");
}

#endif

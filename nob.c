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
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
#define CXX "g++"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c99", "-ggdb", "-O0" //"-O3", "-ffast-math", "-flto", "-s"

void compileExample(Nob_Cmd* cmd, Nob_Procs* procs, char* input, char* output);
void compileAllExample(Nob_Cmd* cmd, Nob_Procs* procs);

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    Nob_Procs procs = {0};

    compileAllExample(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs)) {
        return 1;
    }

    return 0;
}

void compileExample(
    Nob_Cmd*   cmd,
    Nob_Procs* procs,
    char*      input,
    char*      output)
{
    nob_cmd_append(cmd, CC, CFLAGS);
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

void compileAllExample(Nob_Cmd* cmd, Nob_Procs* procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    compileExample(cmd, procs, "examples/array.c", "build/examples/array");
    compileExample(cmd, procs, "examples/arena.c", "build/examples/arena");
    compileExample(cmd, procs, "examples/map.c", "build/examples/map");
    compileExample(cmd, procs, "examples/string.c", "build/examples/string");
    compileExample(cmd, procs, "examples/ringbuf.c", "build/examples/ringbuf");
}

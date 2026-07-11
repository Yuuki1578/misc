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
#define CC  "clang"
#define CXX "clang++"
#define FMT "clang-format"
#elif !defined(__clang__) && defined(__GNUC__)
#define CC  "gcc"
#define CXX "g++"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS "-Wall", "-Werror", "-Wextra", "-pedantic", "-Wno-unused-function", "-ggdb", "-std=c99"

void CompileExample(Nob_Cmd * cmd, Nob_Procs * procs, char *input, char *output);
void CompileAllExample(Nob_Cmd * cmd, Nob_Procs * procs);
void FormatSourceCode(Nob_Cmd * cmd);

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = { 0 };
    Nob_Procs procs = { 0 };

    CompileAllExample(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs)) {
        return 1;
    }

#ifdef __clang__
    if (argc > 1 && strcmp(argv[1], "fmt")) {
        FormatSourceCode(&cmd);
    }
#endif

    return 0;
}

void CompileExample(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output)
{
    nob_cmd_append(cmd, CC, CFLAGS);
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

void CompileAllExample(Nob_Cmd *cmd, Nob_Procs *procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    CompileExample(cmd, procs, "examples/array.c", "build/examples/array");
    CompileExample(cmd, procs, "examples/arena.c", "build/examples/arena");
    CompileExample(cmd, procs, "examples/map.c", "build/examples/map");
}

#ifdef __clang__

#define FORMAT_FILE(file)                                                                                              \
    do {                                                                                                               \
        nob_cmd_append(cmd, FMT, file, "-i");                                                                          \
        nob_cmd_run_async_and_reset(cmd);                                                                              \
    } while (0);

void FormatSourceCode(Nob_Cmd *cmd)
{
    FORMAT_FILE("misc.h");
    FORMAT_FILE("nob.c");
    FORMAT_FILE("examples/array.c");
}

#endif

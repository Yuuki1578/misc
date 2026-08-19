/*
=====================================================================================
DISCLAIMER:
This is a third party build system, all rights reserved to the author of this
library.
=====================================================================================
*/

// Let's integrate our allocator here
#define MISC_IMPL
#include "misc.h"

struct allocator *const alloc = misc_mmap_alloc;

#define NOB_REALLOC(oldptr, size) alloc->realloc(alloc->any, oldptr, size, MISC_ALIGN)
#define NOB_FREE(ptr) alloc->free(alloc->any, ptr)

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

#define CFLAGS "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c99"

void compile_example(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
void compile_all_example(Nob_Cmd *cmd, Nob_Procs *procs);

char *env_opt = NULL;

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = { 0 };
    Nob_Procs procs = { 0 };

    env_opt = getenv("opt");

    compile_all_example(&cmd, &procs);
    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    return 0;
}

void compile_example(
    Nob_Cmd *cmd,
    Nob_Procs *procs,
    char *input,
    char *output)
{
    nob_cmd_append(cmd, CC, CFLAGS);
    if (env_opt != NULL)
        nob_cmd_append(cmd, "-O3", "-ffast-math", "-funroll-loops", "-s", "-flto");
    else
        nob_cmd_append(cmd, "-O0", "-ggdb");

    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
    nob_log(NOB_INFO, "\"%s\" compiled successfuly", input);
}

void compile_all_example(Nob_Cmd *cmd, Nob_Procs *procs)
{
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    compile_example(cmd, procs, "examples/array.c", "build/examples/array");
    compile_example(cmd, procs, "examples/arena.c", "build/examples/arena");
    compile_example(cmd, procs, "examples/map.c", "build/examples/map");
    compile_example(cmd, procs, "examples/string.c", "build/examples/string");
    compile_example(cmd, procs, "examples/ringbuf.c", "build/examples/ringbuf");
    compile_example(cmd, procs, "examples/virtmap.c", "build/examples/virtmap");
    compile_example(cmd, procs, "examples/dump.c", "build/examples/dump");
    compile_example(cmd, procs, "examples/memory_pool.c", "build/examples/memory_pool");
}

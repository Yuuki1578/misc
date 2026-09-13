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

#define NOB_IMPLEMENTATION
#include "nob.h"

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

void compileExample(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
void compileAllExample(Nob_Cmd *cmd, Nob_Procs *procs);

char *env_opt = NULL;

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};
  Nob_Procs procs = {0};

  env_opt = getenv("opt");

  compileAllExample(&cmd, &procs);
  if (!nob_procs_wait_and_reset(&procs))
    return 1;

  return 0;
}

void compileExample(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output) {
  nob_cmd_append(cmd, CC, CFLAGS);
  if (env_opt != NULL)
    nob_cmd_append(cmd, "-O3", "-ffast-math", "-funroll-loops", "-s", "-flto");
  else
    nob_cmd_append(cmd, "-O0", "-ggdb");

  nob_cc_inputs(cmd, input);
  nob_cc_output(cmd, output);
  nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

void compileAllExample(Nob_Cmd *cmd, Nob_Procs *procs) {
  nob_mkdir_if_not_exists("build");
  nob_mkdir_if_not_exists("build/examples");

  compileExample(cmd, procs, "examples/array.c", "build/examples/array");
  compileExample(cmd, procs, "examples/hashmap.c", "build/examples/hashmap");
  compileExample(cmd, procs, "examples/arena.c", "build/examples/arena");
}

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

#define CFLAGS                                                                 \
  "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c23", "-ffast-math",       \
      "-fomit-frame-pointer", "-funroll-loops", "-march=native",               \
      "-mtune=native"

/* ===== EXAMPLES ===== */
static void exampleCompile(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input,
                           char *Output);
static void exampleCompileCpp(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input,
                              char *Output);
static void exampleCompileAll(Nob_Cmd *Cmd, Nob_Procs *Procs);
static void exampleCompileAllCpp(Nob_Cmd *Cmd, Nob_Procs *Procs);
/* ===== EXAMPLES ===== */

/* ===== MISC ===== */
static void sourceFormat(Nob_Cmd *Cmd);
/* ===== MISC ===== */

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd Cmd = {0};
  Nob_Procs Procs = {0};

  exampleCompileAll(&Cmd, &Procs);
  if (!nob_procs_wait_and_reset(&Procs))
    return 1;

  exampleCompileAllCpp(&Cmd, &Procs);
  if (!nob_procs_wait_and_reset(&Procs))
    return 1;

  if (argc > 1) {
    if (strcmp(argv[1], "--format") == 0) {
#ifdef __clang__
      sourceFormat(&Cmd);
#else
      (void)0;
#endif
    } else {
      nob_cmd_append(&Cmd, nob_temp_sprintf("./build/examples/%s", argv[1]));
      if (!nob_cmd_run_sync_and_reset(&Cmd))
        return 1;
    }
  }

  return 0;
}

static void exampleCompile(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input,
                           char *Output) {
  nob_cc(Cmd);
#if defined(__clang__) && defined(ADDRESS_SANITIZER)
  /* It has a builtin sanitizer */
  nob_cmd_append(Cmd, CFLAGS, "-Wno-overlength-strings", "-fsanitize=address");
#else
  /* Can your gcc do that? */
  nob_cmd_append(Cmd, CFLAGS, "-Wno-overlength-strings");
#endif
  nob_cc_inputs(Cmd, Input);
  nob_cc_output(Cmd, Output);
  nob_da_append(Procs, nob_cmd_run_async_and_reset(Cmd));
}

static void exampleCompileCpp(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input,
                              char *Output) {
  nob_cmd_append(Cmd, CXX, Input, "-o", Output,
#if defined(__clang__) && defined(ADDRESS_SANITIZER)
                 "-fsanitize=address",
#endif
                 "-O0", "-ggdb");

  nob_da_append(Procs, nob_cmd_run_async_and_reset(Cmd));
}

static void exampleCompileAll(Nob_Cmd *Cmd, Nob_Procs *Procs) {
  nob_mkdir_if_not_exists("build");
  nob_mkdir_if_not_exists("build/examples");

  exampleCompile(Cmd, Procs, "examples/arena.c", "build/examples/arena");
  exampleCompile(Cmd, Procs, "examples/vector.c", "build/examples/vector");
  exampleCompile(Cmd, Procs, "examples/string.c", "build/examples/string");
  exampleCompile(Cmd, Procs, "examples/refcount.c", "build/examples/refcount");
  exampleCompile(Cmd, Procs, "examples/list.c", "build/examples/list");
  exampleCompile(Cmd, Procs, "examples/file_reading.c",
                 "build/examples/file_reading");
  exampleCompile(Cmd, Procs, "examples/numeric.c", "build/examples/numeric");
  exampleCompile(Cmd, Procs, "examples/file.c", "build/examples/file");
  exampleCompile(Cmd, Procs, "examples/linked_list.c",
                 "build/examples/linked_list");
  exampleCompile(Cmd, Procs, "examples/double_link.c",
                 "build/examples/double_link");
  exampleCompile(Cmd, Procs, "examples/raw_dlink.c",
                 "build/examples/raw_dlink");
  exampleCompile(Cmd, Procs, "examples/hexdump.c", "build/examples/hexdump");
}

static void exampleCompileAllCpp(Nob_Cmd *Cmd, Nob_Procs *Procs) {
  nob_mkdir_if_not_exists("build");
  nob_mkdir_if_not_exists("build/examples");

  exampleCompileCpp(Cmd, Procs, "examples/dlink_binding.cc",
                    "build/examples/dlink_binding");
}

#ifdef __clang__

#define formatFile(File)                                                       \
  do {                                                                         \
    nob_cmd_append(Cmd, FMT, File, "-i");                                      \
    nob_cmd_run_async_and_reset(Cmd);                                          \
  } while (0);

static void sourceFormat(Nob_Cmd *Cmd) {
  formatFile("examples/arena.c");
  formatFile("examples/refcount.c");
  formatFile("examples/string.c");
  formatFile("examples/vector.c");
  formatFile("examples/file_reading.c");
  formatFile("examples/numeric.c");
  formatFile("examples/file.c");
  formatFile("examples/linked_list.c");
  formatFile("examples/double_link.c");
  formatFile("examples/raw_dlink.c");
  formatFile("examples/dlink_binding.cc");
  formatFile("examples/hexdump.c");

  formatFile("nob.c");
}

#endif

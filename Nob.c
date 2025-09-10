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

#define CFLAGS                                                           \
  "-Wall", "-Werror", "-Wextra", "-pedantic", "-std=c23", "-ffast-math", \
      "-fomit-frame-pointer", "-funroll-loops", "-march=native",         \
      "-mtune=native"

/* ===== EXAMPLES ===== */
static void exampleCompile(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input, char *Output);
static void exampleCompileCpp(Nob_Cmd *Cmd, Nob_Procs *Procs, char *Input, char *Output);
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

  exampleCompile(Cmd, Procs, "examples/Arena.c", "build/examples/Arena");
  exampleCompile(Cmd, Procs, "examples/Vector.c", "build/examples/Vector");
  exampleCompile(Cmd, Procs, "examples/String.c", "build/examples/String");
  exampleCompile(Cmd, Procs, "examples/RefCount.c", "build/examples/RefCount");
  exampleCompile(Cmd, Procs, "examples/List.c", "build/examples/List");
  exampleCompile(Cmd, Procs, "examples/FileReading.c", "build/examples/FileReading");
  exampleCompile(Cmd, Procs, "examples/Numeric.c", "build/examples/Numeric");
  exampleCompile(Cmd, Procs, "examples/File.c", "build/examples/File");
  exampleCompile(Cmd, Procs, "examples/LinkedList.c", "build/examples/LinkedList");
  exampleCompile(Cmd, Procs, "examples/DoubleLink.c", "build/examples/DoubleLink");
  exampleCompile(Cmd, Procs, "examples/RawDlink.c", "build/examples/RawDlink");
  exampleCompile(Cmd, Procs, "examples/Hexdump.c", "build/examples/Hexdump");
}

static void exampleCompileAllCpp(Nob_Cmd *Cmd, Nob_Procs *Procs) {
  nob_mkdir_if_not_exists("build");
  nob_mkdir_if_not_exists("build/examples");

  exampleCompileCpp(Cmd, Procs, "examples/DlinkBinding.cc", "build/examples/DlinkBinding");
}

#ifdef __clang__

#define formatFile(File)                  \
  do {                                    \
    nob_cmd_append(Cmd, FMT, File, "-i"); \
    nob_cmd_run_async_and_reset(Cmd);     \
  } while (0);

static void sourceFormat(Nob_Cmd *Cmd) {
  formatFile("Misc.h");
  formatFile("examples/Arena.c");
  formatFile("examples/RefCount.c");
  formatFile("examples/String.c");
  formatFile("examples/Vector.c");
  formatFile("examples/FileReading.c");
  formatFile("examples/Numeric.c");
  formatFile("examples/File.c");
  formatFile("examples/LinkedList.c");
  formatFile("examples/DoubleLink.c");
  formatFile("examples/RawDlink.c");
  formatFile("examples/DlinkBinding.cc");
  formatFile("examples/Hexdump.c");

  formatFile("Nob.c");
}

#endif

/*

======= Copyright (c) 2024 Alexey Kutepov =======
         Licensed under the MIT License

The Fuck Around and Find Out License v0.1
Copyright (C) 2025 Awang Destu Pradhana

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:

1. The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the software.

2. The software shall be used for Good, not Evil. The original author of the
software retains the sole and exclusive right to determine which uses are
Good and which uses are Evil.

3. The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. In no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software. */

#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#ifdef __clang__
#define CC "clang"
#define AR "llvm-ar"
#elif !defined(__clang__) && defined(__GNUC__)
#define CC "gcc"
#define AR "ar"
#else
#error Compiler must be either gcc or clang
#endif

#define CFLAGS               \
    "-Wall",                 \
    "-Werror",               \
    "-Wextra",               \
    "-std=c23",              \
    "-pedantic",             \
    "-ffast-math",           \
    "-fomit-frame-pointer",  \
    "-funroll-loops",        \
    "-march=native",         \
    "-mtune=native"

#define ARFLAGS "rcs"

/* ===== CORE FILES ===== */
static void core_compile_only(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
static void core_compile_only_all(Nob_Cmd *cmd, Nob_Procs *procs);
static void core_create_archive(Nob_Cmd *cmd, Nob_Procs *procs);
/* ===== CORE FILES ===== */

/* ===== EXAMPLES ===== */
static void examples_compile(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output);
static void examples_compile_all(Nob_Cmd *cmd, Nob_Procs *procs);
/* ===== EXAMPLES ===== */

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    Nob_Procs procs = {0};

    core_compile_only_all(&cmd, &procs);
    core_create_archive(&cmd, &procs);

    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    examples_compile_all(&cmd, &procs);

    if (!nob_procs_wait_and_reset(&procs))
        return 1;

    return 0;
}

static void core_compile_only(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output)
{
    nob_cc(cmd);
    nob_cmd_append(cmd, CFLAGS, "-c");
    nob_cc_inputs(cmd, input);
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void core_compile_only_all(Nob_Cmd *cmd, Nob_Procs *procs)
{
    nob_mkdir_if_not_exists("build");

    core_compile_only(cmd, procs, "src/arena.c", "build/arena.o");
    core_compile_only(cmd, procs, "src/vector.c", "build/vector.o");
    core_compile_only(cmd, procs, "src/string.c", "build/string.o");
    core_compile_only(cmd, procs, "src/refcount.c", "build/refcount.o");
}

static void core_create_archive(Nob_Cmd *cmd, Nob_Procs *procs)
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
        "build/refcount.o"
        /* ===== OBJECT FILES ===== */
    );

    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void examples_compile(Nob_Cmd *cmd, Nob_Procs *procs, char *input, char *output)
{
    nob_cc(cmd);
    nob_cmd_append(cmd, CFLAGS, "-Wno-overlength-strings");
    nob_cc_inputs(cmd, input, "build/libmisc.a");
    nob_cc_output(cmd, output);
    nob_da_append(procs, nob_cmd_run_async_and_reset(cmd));
}

static void examples_compile_all(Nob_Cmd *cmd, Nob_Procs *procs)
{
    nob_mkdir_if_not_exists("build/examples");
    
    examples_compile(cmd, procs, "examples/arena.c", "build/examples/arena");
    examples_compile(cmd, procs, "examples/vector.c", "build/examples/vector");
    examples_compile(cmd, procs, "examples/string.c", "build/examples/string");
    examples_compile(cmd, procs, "examples/refcount.c", "build/examples/refcount");
    examples_compile(cmd, procs, "examples/list.c", "build/examples/list");
}

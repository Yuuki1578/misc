/*
======= Copyright (c) 2024 Alexey Kutepov =======
         Licensed under the MIT License
*/

/* The Fuck Around and Find Out License v0.1
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

#ifdef _MSVC_VER
#    error MSVC is not supported
#endif

#ifdef __clang__
#    define CC "clang"
#    define AR "llvm-ar"
#elif defined(__GNUC__) && !defined(__clang__)
#    define CC "gcc"
#    define AR "ar"
#endif

#define CFLAGS                                                                 \
    "-Wall", "-Werror", "-Wextra", "-std=c11", "-pedantic", "-ffast-math",     \
        "-fomit-frame-pointer", "-funroll-loops", "-O2", "-march=native",      \
        "-mtune=native"

#define ARFLAGS    "rcs"
#define STATIC_LIB "build/libmisc.a"

static char *srcs[] = {
    "src/arena.c",
    "src/string.c",
    "src/vector.c",
    "src/reference_counting.c",
};

static char *objs[] = {
    "build/arena.o",
    "build/string.o",
    "build/vector.o",
    "build/reference_counting.o",
};

static char *examples[] = {
    "examples/arena.c",
    "examples/string.c",
    "examples/vector.c",
    "examples/reference_counting.c",
};

static char *exe_amples[] = {
    "build/examples/arena",
    "build/examples/string",
    "build/examples/vector",
    "build/examples/reference_counting",
};

static size_t file_total = sizeof srcs / sizeof *srcs;

void compile_source_files(Nob_Cmd *cmd)
{
    for (size_t i = 0; i < file_total; i++) {
        nob_cmd_append(cmd, CC, CFLAGS, srcs[i], "-c", "-o", objs[i]);
        nob_cmd_run_sync_and_reset(cmd);
    }
}

void archive_object_files(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, AR, ARFLAGS, STATIC_LIB, objs[0], objs[1], objs[2],
                   objs[3]);
    nob_cmd_run_sync_and_reset(cmd);
}

void create_examples(Nob_Cmd *cmd)
{
    for (size_t i = 0; i < file_total; i++) {
        nob_cmd_append(cmd, CC, CFLAGS, "-fsanitize=address",
                       "-Wno-overlength-strings", "-Lbuild", "-lmisc",
                       examples[i], "-o", exe_amples[i]);
        nob_cmd_run_sync_and_reset(cmd);
    }
}

int main(int argc, char **argv)
{
    Nob_Cmd cmd = {0};

    NOB_GO_REBUILD_URSELF(argc, argv);
    nob_mkdir_if_not_exists("build");
    nob_mkdir_if_not_exists("build/examples");

    compile_source_files(&cmd);
    archive_object_files(&cmd);
    create_examples(&cmd);

    nob_cmd_free(cmd);
    return 0;
}

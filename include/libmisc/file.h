#ifndef MISC_FILE_H
#define MISC_FILE_H

#include <stdio.h>

char* file_readall(const char* path);
char* file_readfrom(FILE* file);

#endif

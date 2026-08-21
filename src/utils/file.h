#ifndef FILE_H
#define FILE_H

#include <stdio.h>

struct Arena;

FILE* file_openw(const char* path);
const char* file_read(struct Arena* a, const char* path);

#endif

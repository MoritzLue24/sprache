#ifndef FILE_H
#define FILE_H

#include "utils/arena.h"
#include <stdio.h>

FILE* openw_file(const char* path);
const char* read_file(struct Arena* a, const char* path);

#endif

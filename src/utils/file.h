#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include "utils/arena.h"

FILE*       openw_file(const char* path);
const char* read_file(struct Arena* a, const char* path);

#endif
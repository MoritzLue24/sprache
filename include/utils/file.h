#ifndef FILE_H
#define FILE_H

#include <stdbool.h>


char* read_file(const char* path);

bool file_exists(const char* path);

#endif
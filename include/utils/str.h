#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include <stdlib.h>


/// @note needs freeing 
char* replace_last(const char* string, const char* substr, const char* new_substr);

bool ends_with(const char* string, const char* suffix);

/// @note needs freeing 
char* substr(const char* string, size_t start, size_t end);

#endif
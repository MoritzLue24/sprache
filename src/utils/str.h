#ifndef STR_H
#define STR_H

#include <stdbool.h>

struct Arena;

/// @brief Modifies 's' to UPPERCASE, also returns the new s.
char* str_upper(char* s);
bool str_ends_with(const char* s, const char* suffix);
char* str_replace_last(
    struct Arena* a, const char* s, const char* substr, const char* new_substr
);

#endif

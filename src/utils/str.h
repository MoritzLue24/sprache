#ifndef STR_H
#define STR_H

#include "utils/arena.h"
#include <stdbool.h>

/// @brief Modifies 's' to UPPERCASE, also returns the new s.
char* strupper(char* s);
bool ends_with(const char* s, const char* suffix);
char* replace_last(
    struct Arena* a, const char* s, const char* substr, const char* new_substr
);

#endif

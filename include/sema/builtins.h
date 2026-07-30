#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdlib.h>


enum BuiltinType {
    BUILTIN_READ_PIN
};

struct BuiltinDef {
    enum BuiltinType type; 
    const char* ident;
    size_t arg_count;
};

extern const struct BuiltinDef builtins[];
extern const size_t builtins_count;

/// @brief Returns a pointer to the BuiltinDef matching with the ident
const struct BuiltinDef* match_builtin(const char* ident);

/// @brief prints the given builtin definition with the given depth as indentation, and with a label in front (NULL for nothing). Prints label: <unresolved> if def == NULL
void print_builtin_def(const char* label, const struct BuiltinDef* def, int depth);

#endif
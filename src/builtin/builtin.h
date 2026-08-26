#ifndef BUILTIN_H
#define BUILTIN_H

#include "type/type.h"
#include <stddef.h>

enum BuiltinKind {
    BUILTIN_INVALID,
#define BUILTIN(kind, ...) kind,
#include "builtin/builtin.def"
#undef BUILTIN
};

/// @returns the builtin kind as a string.
/// On invalid builtin (not BUILTIN_INVALID), return NULL.
const char* builtin_kind_str(enum BuiltinKind kind);
enum BuiltinKind builtin_kind_from_spelling(const char* spelling);
enum TypeKind builtin_kind_type(enum BuiltinKind kind);
size_t builtin_kind_argc(enum BuiltinKind kind);
/// @returns the type of the arg at position i (starts at 0).
enum TypeKind builtin_kind_arg_type_at(enum BuiltinKind kind, size_t i);

#endif
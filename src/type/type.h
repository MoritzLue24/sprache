#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>
#include <stdbool.h>

enum TypeKind {
    TYPE_INVALID,
#define TYPE(kind, spelling, size, is_signed) kind,
#include "type/type.def"
#undef TYPE
};

/// @returns the type kind as a string.
/// On invalid kind (not TYPE_INVALID), returns NULL.
const char* type_kind_str(enum TypeKind type);
/// @returns the first match with spelling.
/// TYPE_INVALID on no match.
enum TypeKind type_kind_from_spelling(const char* spelling);

/// @returns the size of the type, in bytes  
size_t type_size(enum TypeKind type);
bool type_is_signed(enum TypeKind type);
/// @brief Determines whenever the given literal fits inside the type 
bool type_fits(enum TypeKind type, const char* s);

#endif
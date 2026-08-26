#include "builtin/builtin.h"
#include <string.h>

const char* builtin_kind_str(enum BuiltinKind kind)
{
    switch (kind) {
        case BUILTIN_INVALID: return "BUILTIN_INVALID";
#define BUILTIN(kind, ...) case kind: return #kind;
#include "builtin/builtin.def"
#undef BUILTIN
    }
    return NULL;
}

enum BuiltinKind builtin_kind_from_spelling(const char* spelling)
{
#define BUILTIN(kind, spelling_, ...) if (strcmp(spelling_, spelling) == 0) { \
    return kind; \
}
#include "builtin/builtin.def"
#undef BUILTIN
    return BUILTIN_INVALID;
}

enum TypeKind builtin_kind_type(enum BuiltinKind kind)
{
    switch (kind) {
        case BUILTIN_INVALID: return TYPE_INVALID;
#define BUILTIN(kind, spelling, type, ...) case kind: return type;
#include "builtin/builtin.def"
#undef BUILTIN
    }
    return TYPE_INVALID;
}

size_t builtin_kind_argc(enum BuiltinKind kind)
{
    switch (kind) {
        case BUILTIN_INVALID: return 0;
#define BUILTIN(kind, spelling, type, argc, ...) case kind: return argc;
#include "builtin/builtin.def"
#undef BUILTIN
    }
    return 0;
}

enum TypeKind builtin_kind_arg_type_at(enum BuiltinKind kind, size_t i)
{
    switch (kind) {
        case BUILTIN_INVALID: return TYPE_INVALID;
#define BUILTIN(kind, spelling, type, argc, a0, a1, a2, a3) case kind: { \
    if (i >= argc) return TYPE_INVALID; \
    return ((enum TypeKind[]){ a0, a1, a2, a3 })[i]; \
}
#include "builtin/builtin.def"
#undef BUILTIN
    }
    return TYPE_INVALID;
}
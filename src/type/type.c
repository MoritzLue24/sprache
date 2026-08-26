#include "type/type.h"
#include <string.h>

const char* type_kind_str(enum TypeKind type)
{
    switch (type) {
        case TYPE_INVALID: return "TYPE_INVALID";
#define TYPE(kind, ...) case kind: return #kind;
#include "type/type.def"
#undef TYPE
    }
    return NULL;
}

enum TypeKind type_kind_from_spelling(const char* spelling)
{
#define TYPE(kind, spelling_, ...) if (strcmp(spelling, spelling_) == 0) \
    return kind;
#include "type/type.def"
#undef TYPE
    return TYPE_INVALID;
}
  
size_t type_size(enum TypeKind type)
{
    switch (type) {
        case TYPE_INVALID: return 0;
#define TYPE(kind, spelling, size, is_signed) case kind: return size;
#include "type/type.def"
#undef TYPE
    }
    return 0;
}

bool type_is_signed(enum TypeKind type)
{
    switch (type) {
        case TYPE_INVALID: return false;
#define TYPE(kind, spelling, size, is_signed) case kind: return is_signed;
#include "type/type.def"
#undef TYPE
    }
    return false;
}

bool type_fits(enum TypeKind type, const char* s)
{
    // TODO
    return false;
}
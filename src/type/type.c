#include "type/type.h"
#include <string.h>
// @claude-begin
#include <stdlib.h>
// @claude-end
#include <stdint.h>
#include <errno.h>

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

const char* type_kind_spelling(enum TypeKind type)
{
    switch (type) {
        case TYPE_INVALID: return NULL;
#define TYPE(kind, spelling, ...) case kind: return spelling;
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
    size_t size = type_size(type);
    if (size == 0 || size > sizeof(uint64_t))
        return false;

    errno = 0;
    char* end;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno == ERANGE || *end != '\0')
        return false;

    size_t bits = size * 8 - (type_is_signed(type) ? 1 : 0);
    uint64_t max = bits >= 64 ? UINT64_MAX : ((uint64_t)1 << bits) - 1;
    return v <= max;
}
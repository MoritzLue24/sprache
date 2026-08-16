#include "core/type.h"

#include <assert.h>


const char* type_str(enum Type t)
{
    switch (t) {
        case TYPE_INVALID: return "INVALID";
        case TYPE_UINT8: return "UINT8";
        case TYPE_INT8: return "INT8";
    }
    assert(0);
    return 0;
}

size_t type_size(enum Type t)
{
    switch (t) {
        case TYPE_INVALID: break;
        case TYPE_UINT8:
        case TYPE_INT8: return 1;
    }
    assert(0);
    return 0;
}

bool type_is_signed(enum Type t)
{
    switch (t) {
        case TYPE_INVALID: break;
        case TYPE_UINT8: return false;
        case TYPE_INT8: return true;
    }
    assert(0);
    return false;
}

bool type_fits(enum Type t, long value)
{
    switch (t) {
        case TYPE_INVALID: break;
        case TYPE_UINT8:
            return TYPE_UINT8_MIN <= value
                && value <= TYPE_UINT8_MAX;
        case TYPE_INT8:
            return TYPE_INT8_MIN <= value
                && value <= TYPE_INT8_MAX;
    }
    assert(0);
    return false;
}
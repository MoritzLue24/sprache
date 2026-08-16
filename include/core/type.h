#ifndef TYPE_H
#define TYPE_H

#include <stdlib.h>
#include <stdbool.h>

#define TYPE_UINT8_MIN 0
#define TYPE_UINT8_MAX 255
#define TYPE_INT8_MIN -128
#define TYPE_INT8_MAX 127


enum Type {
    TYPE_INVALID,
    TYPE_UINT8,
    TYPE_INT8
};

const char* type_str(enum Type t);

size_t type_size(enum Type t);

bool type_is_signed(enum Type t);

bool type_fits(enum Type t, long value);

#endif
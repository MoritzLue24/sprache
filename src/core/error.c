#include "frontend/core/error.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils/xalloc.h"


static void expand_errorlist(struct ErrorList* errors)
{
    errors->capacity *= 2;
    errors->data = xrealloc(errors->data, errors->capacity, sizeof(struct Error));
    assert(errors->data);
}

void init_errorlist(struct ErrorList* errors, size_t capacity)
{
    errors->data = xcalloc(capacity, sizeof(struct Error));
    errors->capacity = capacity;
    errors->size = 0;
}

bool has_errors(const struct ErrorList* errors)
{
    return errors->size > 0;
}

void add_error(struct ErrorList* errors, enum ErrorType type, struct Loc loc, const char* format, ...)
{
    assert(errors);
    if (errors->size + 1 >= errors->capacity)
        expand_errorlist(errors);

    va_list args;
    va_start(args, format);

    // calculate message length
    va_list copy;
    va_copy(copy, args);

    size_t length = vsnprintf(NULL, 0, format, copy);   // returns length without writing anywhere
    va_end(copy);

    // write message
    char *message = xmalloc(length + 1);
    vsnprintf(message, length + 1, format, args);   // sets nullbyte automatically
    va_end(args);

    errors->data[errors->size].type = type;
    errors->data[errors->size].loc = loc;
    errors->data[errors->size].message = message;
    errors->size++;
}

void print_errors(const struct ErrorList* errors)
{
    if (errors->size == 0)
    {
        printf("No errors\n");
        return;
    }

    for (size_t i = 0; i < errors->size; i++)
    {
        struct Error error = errors->data[i];
        const char* type;
        switch (error.type)
        {
            case ERROR_TOKEN:
                type = "TOKEN";
                break;
            case ERROR_SYNTAX:
                type = "SYNTAX";
                break;
            case ERROR_UNDECLARED:
                type = "UNDECLARED";
                break;
            case ERROR_REDECLARATION:
                type = "REDECLARATION";
                break;
            case ERROR_LVALUE_NOT_MODIFIABLE:
                type = "LVALUE_NOT_MODIFIABLE";
                break;
            case ERROR_NOT_CALLABLE:
                type = "NOT_CALLABLE";
                break;
            case ERROR_INVALID_BUILTIN:
                type = "INVALID_BUILTIN";
                break;
            case ERROR_INVALID_ARG_SIZE:
                type = "INVALID_ARG_SIZE";
                break;
            default:
                assert(0);
        }
        printf("At line %i, column %i:\n\t%s: \"%s\"\n", error.loc.ln, error.loc.col, type, error.message);
    }
}

void free_errorlist(struct ErrorList* errors)
{
    for (size_t i = 0; i < errors->size; i++)
        xfree((void**)&errors->data[i].message);
    xfree((void**)&errors->data);
}
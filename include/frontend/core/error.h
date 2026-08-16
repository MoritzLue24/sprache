#ifndef ERROR_H
#define ERROR_H

#include "frontend/core/loc.h"

#define ERRORLIST_INIT_CAPACITY 10


enum ErrorType {
    ERROR_TOKEN,
    ERROR_SYNTAX,
    ERROR_UNDECLARED,
    ERROR_REDECLARATION,
    ERROR_LVALUE_NOT_MODIFIABLE,
    ERROR_NOT_CALLABLE,
    ERROR_NOT_A_VALUE,
    ERROR_INVALID_BUILTIN,
    ERROR_INVALID_ARG_SIZE,
    ERROR_MAIN_MISSING
};

struct Error {
    enum ErrorType type;
    struct Loc loc;
    char* message;
};

struct ErrorList {
    struct Error* data;
    size_t capacity;
    size_t size;
};

void init_errorlist(struct ErrorList* errors);

bool has_errors(const struct ErrorList* errors);

void add_error(struct ErrorList* errors, enum ErrorType type, struct Loc loc, const char* format, ...)
    __attribute__((format(printf, 4, 5)));

void print_errors(const struct ErrorList* errors);

void free_errorlist(struct ErrorList* errors);

#endif
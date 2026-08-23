#include "sprache/diag.h"
#include "diag/diag_internal.h"
#include "utils/darray.h"
#include <string.h>
#include <stdarg.h>

static const char* diag_code_format(enum DiagCode code);

const char* diag_code_str(enum DiagCode code)
{
    switch (code) {
        case DIAG_INVALID: return "DIAG_INVALID";
#define DIAG(name, format) case name: return #name;
#include "sprache/diag.def"
#undef DIAG
    }
    return NULL;
}

bool diag_has_errors(const struct DiagList* dl)
{
    return dl->count > 0;
}

void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
) {
    va_list args;
    va_start(args, loc);
    diag_vadd(a, dl, code, loc, args);
    va_end(args);
}

void diag_vadd(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, va_list args
) {
    const char* format = diag_code_format(code);

    // calculate message length
    va_list copy;
    va_copy(copy, args);

    // returns length without writing anywhere
    size_t length = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    // write message
    char* message = ARENA_CALLOC_LIST(a, length + 1, char);
    // sets nullbyte automatically
    vsnprintf(message, length + 1, format, args);

    DARRAY_ADD(
        a, dl, ((struct Diag){ .code = code, .loc = loc, .message = message })
    );
}

static const char* diag_code_format(enum DiagCode code)
{
    switch (code) {
        case DIAG_INVALID:
            break;
#define DIAG(name, format) case name: return format;
#include "sprache/diag.def"
#undef DIAG
    }
    return NULL;
}
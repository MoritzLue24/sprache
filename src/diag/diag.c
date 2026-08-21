#include "sprache/diag.h"
#include "diag/diag_internal.h"
#include "utils/darray.h"
#include <string.h>
#include <stdarg.h>

#define DIAG_INIT_CAPACITY 10

const char* diag_code_str(enum DiagCode code)
{
    switch (code) {
        case DIAG_INVALID:
            break;
#define DIAG(name, format) case name: return #name;
#include "sprache/diag.def"
#undef DIAG
    }
    return "<invalid diag code>";
}

bool diag_has_errors(const struct DiagList* dl)
{
    return dl->count > 0;
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

void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
) {
    const char* format = diag_code_format(code);

    va_list args;
    va_start(args, loc);

    // calculate message length
    va_list copy;
    va_copy(copy, args);

    size_t length = vsnprintf(
        NULL, 0, format,
        copy
    ); // returns length without writing anywhere
    va_end(copy);

    // write message
    char* message = ARENA_CALLOC_LIST(a, length + 1, char);
    vsnprintf(message, length + 1, format, args); // sets nullbyte automatically
    va_end(args);

    DARRAY_ADD(
        a, dl, ((struct Diag){
            .code = code,
            .loc = loc,
            .message = message
        })
    );
}

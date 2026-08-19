#include "sprache/diag.h"
#include "diag/diag_internal.h"
#include <string.h>
#include <stdarg.h>

#define DIAG_INIT_CAPACITY 10

bool diag_has_errors(const struct DiagList* dl)
{
    return dl->count > 0;
}

const char* diag_code_str(enum DiagCode code)
{
    switch (code) {
        case DIAG_INVALID: break;
#define DIAG(name, format) case name: return #name;
#include "sprache/diag.def"
#undef DIAG
    }
    return "<invalid diag code>";
}

void diag_list_init(struct Arena* a, struct DiagList* dl)
{
    dl->items    = ARENA_CALLOC_LIST(a, DIAG_INIT_CAPACITY, struct Diag);
    dl->count    = 0;
    dl->capacity = DIAG_INIT_CAPACITY;
}

static const char* diag_code_format(enum DiagCode code)
{
    switch (code) {
        case DIAG_INVALID: break;
#define DIAG(name, format) case name: return format;
#include "sprache/diag.def"
#undef DIAG
    }
    return NULL;
}

void diag_add(struct Arena* a, struct DiagList* dl, enum DiagCode code,
              struct SourceLoc loc, ...)
{
    if (dl->count >= dl->capacity) {
        size_t new_capacity    = dl->capacity * 2;
        struct Diag* new_items = ARENA_CALLOC_LIST(a, new_capacity, struct Diag);
       
        memcpy(new_items, dl->items, dl->count * sizeof(struct Diag));

        dl->items    = new_items;
        dl->capacity = new_capacity;
    }

    const char* format = diag_code_format(code);

    va_list args;
    va_start(args, format);

    // calculate message length
    va_list copy;
    va_copy(copy, args);

    size_t length = vsnprintf(NULL, 0, format, copy);   // returns length without writing anywhere
    va_end(copy);

    // write message
    char* message = ARENA_CALLOC_LIST(a, length + 1, char);
    vsnprintf(message, length + 1, format, args);   // sets nullbyte automatically
    va_end(args);

    dl->items[dl->count++] = (struct Diag){
        .code = code,
        .loc = loc,
        .message = message
    };
}
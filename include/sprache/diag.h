#ifndef DIAG_H
#define DIAG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define SOURCE_LOC_NULL ((struct SourceLoc){ .line = 0, .col = 0 })

enum DiagCode {
    DIAG_INVALID,
#define DIAG(name, format) name,
#include "sprache/diag.def"
#undef DIAG
};

/// @returns the diag code as a string.
/// On invalid diag code (not DIAG_INVALID), return NULL.
const char* diag_code_str(enum DiagCode code);

struct SourceLoc {
    unsigned line;
    unsigned col;
};

struct Diag {
    enum DiagCode code;
    struct SourceLoc loc;
    const char* message;
};

struct DiagList {
    struct Diag* items;
    size_t count;
    size_t capacity;
};

bool diag_has_errors(const struct DiagList* dl);
void diag_dump(const struct Diag* d, FILE* out);
void diag_dump_all(const struct DiagList* dl, FILE* out);

#endif

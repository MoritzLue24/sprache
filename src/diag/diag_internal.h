#ifndef DIAG_INTERNAL_H
#define DIAG_INTERNAL_H

#include "sprache/diag.h"
#include <stdarg.h>

#define DIAG_INIT_CAPACITY 10

struct Arena;

/// @brief Adds a formatted diagnostic using the format specified in "diag.def"
void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
);

/// @brief Like diag_add, but with an already started argument list.
/// @note Does not end 'args', that stays the caller's job.
void diag_vadd(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, va_list args
);

#endif

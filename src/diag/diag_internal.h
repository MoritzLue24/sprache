#ifndef DIAG_INTERNAL_H
#define DIAG_INTERNAL_H

#include "sprache/diag.h"
#include "utils/arena.h"

/// @brief Adds a formatted diagnostic using the format specified in "diag.def"
void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
);

#endif

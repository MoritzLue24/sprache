#ifndef DIAG_INTERNAL_H
#define DIAG_INTERNAL_H

#include "sprache/diag.h"
#include "utils/arena.h"

void diag_add(
    struct Arena* a, struct DiagList* dl, enum DiagCode code,
    struct SourceLoc loc, ...
);

#endif
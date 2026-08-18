#ifndef DIAG_INTERNAL_H
#define DIAG_INTERNAL_H

#include "sprache/diag.h"

void diag_list_init(struct DiagList* dl);
void diag_list_free(struct DiagList* dl);

void diag_add(struct DiagList* dl, enum DiagCode code,
              struct SourceLoc loc, ...);

#endif
#include "sprache/diag.h"
#include "diag/diag_internal.h"

bool diag_has_errors(const struct DiagList* dl)
{
    return dl->count > 0;
}

const char* diag_code_str(enum DiagCode code)
{
    switch (code) {
#define DIAG(name, format) case name: return "#name";
#include "sprache/diag.def"
#undef DIAG
    }
    return "<invalid diag code>";
}

void diag_list_init(struct DiagList* dl)
{
    
}

void diag_list_free(struct DiagList* dl);

void diag_add(struct DiagList* dl, enum DiagCode code,
              struct SourceLoc loc, ...);
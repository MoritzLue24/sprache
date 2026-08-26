#include "sema/symbols.h"

const char* symbol_kind_str(enum SymbolKind sym_kind)
{
    switch (sym_kind) {
        case SYM_INVALID: return "SYM_INVALID";
        case SYM_TYPE: return "SYM_TYPE";
        case SYM_BUILTIN: return "SYM_BUILTIN";
        case SYM_FUNC: return "SYM_FUNC";
        case SYM_VAR: return "SYM_VAR";
    }
    return NULL;
}
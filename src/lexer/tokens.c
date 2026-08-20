#include "lexer/tokens.h"

#include <string.h>

const char* token_kind_str(enum TokenKind tok_kind)
{
    switch (tok_kind) {
        case TK_INVALID: return "TK_INVALID";
#define TOKEN(kind, spelling, class) case kind: return #kind;
#include "lexer/tokens.def"
#undef TOKEN
    }
    return "TK_INVALID";
}

enum TokenKind str_token_kind(const char* s)
{
#define TOKEN(kind, spelling, class) if (strcmp(s, spelling) == 0) { \
    return kind; }
#include "lexer/tokens.def"
#undef TOKEN
    return TK_INVALID;
}

bool token_kind_is_kw(enum TokenKind tok_kind)
{
#define TOKEN(kind, spelling, class) if (class == TC_KW \
    && kind == tok_kind) return true;
#include "lexer/tokens.def"
#undef TOKEN
    return false;
}

bool token_kind_is_punct(enum TokenKind tok_kind)
{
#define TOKEN(kind, spelling, class) if (class == TC_PUNCT \
    && kind == tok_kind) return true;
#include "lexer/tokens.def"
#undef TOKEN
    return false;
}


#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include "sprache/diag.h"

enum TokenClass {
    TC_OTHER,
    TC_KW,
    TC_PUNCT
};

enum TokenKind {
    TK_INVALID,
#define TOKEN(kind, spelling, class) kind,
#include "lexer/tokens.def"
#undef TOKEN
};

const char*    token_kind_str(enum TokenKind tok_kind);
enum TokenKind str_token_kind(const char* s);
bool           token_kind_is_kw(enum TokenKind tok_kind);
bool           token_kind_is_punct(enum TokenKind tok_kind);

struct Token {
    enum TokenKind   kind;
    const char*      value;
    struct SourceLoc loc;
};

struct TokenList {
    struct Token* items;
    size_t        count;
    size_t        capacity;
};

void dump_token(const struct Token* tok, FILE* out);
void dump_all_tokens(const struct TokenList* tokl, FILE* out);

#endif
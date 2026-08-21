#ifndef TOKENS_H
#define TOKENS_H

#include "sprache/diag.h"
#include <stdio.h>

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

const char* token_kind_str(enum TokenKind tok_kind);
enum TokenKind token_kind_from_str(const char* s);
bool token_kind_is_kw(enum TokenKind tok_kind);
bool token_kind_is_punct(enum TokenKind tok_kind);

struct Token {
    enum TokenKind kind;
    const char* value;
    struct SourceLoc loc;
};

struct TokenList {
    struct Token* items;
    size_t count;
    size_t capacity;
};

void token_dump(const struct Token* tok, FILE* out);
void token_dump_all(const struct TokenList* tokl, FILE* out);

#endif

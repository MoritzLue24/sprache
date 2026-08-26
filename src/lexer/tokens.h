#ifndef TOKENS_H
#define TOKENS_H

#include "sprache/diag.h"
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#define TOKENLIST_INIT_CAPACITY 50
#define TOKEN_NULL ((struct Token){ \
    .kind = TK_INVALID, \
    .value = NULL, \
    .loc = SOURCE_LOC_NULL \
})

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

/// @returns the Token kind as a string.
/// On invalid builtin (not TK_INVALID), return NULL.
const char* token_kind_str(enum TokenKind tok_kind);
/// @returns the first token that matches the spelling.
/// TK_INVALID on no match.
enum TokenKind token_kind_from_spelling(const char* s);
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

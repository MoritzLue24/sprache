#ifndef TOKENS_H
#define TOKENS_H

#include <stdlib.h>
#include "core/loc.h"
#include "core/match_type_pair.h"


/// @note Contains all token types, including all keywords & punctuations
enum TokenType {
    TT_INVALID,
    TT_END,

    TT_IDENT,

    TT_VAR,
    TT_RETURN,

    TT_SEMICOLON,
    TT_COMMA,
    TT_AT,
    TT_EQ,
    TT_LPAREN,
    TT_RPAREN,
    TT_PLUS,
    TT_MINUS,
    TT_STAR,

    TT_LITERAL,
};

struct Token {
	enum TokenType type;
	char* value;    // not const because of free
    struct Loc begin;
    // struct Loc end;
    struct Token* next;
};

extern const struct MatchTypePair punctuations[];
extern const size_t punctuations_count;

extern const struct MatchTypePair keywords[];
extern const size_t keywords_count;


/// @brief Converts the `enum TokenType` to a string by matching
const char* tt_str(enum TokenType tt);

/// @brief Prints a linked-tokenlist as the following format:
/// <type>: <line>:<col>, '<value_if_exists>'
void print_tokenlist(struct Token* head);

void free_tokenlist(struct Token* head);

#endif
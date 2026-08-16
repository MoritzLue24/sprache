#ifndef TOKENS_H
#define TOKENS_H

#include <stdlib.h>

#include "frontend/core/loc.h"


/// @note Contains all token types, including all keywords & punctuations
enum TokenType {
    // general
    TT_INVALID,
    TT_END,
    TT_IDENT,
    TT_LITERAL,

    // keywords
    TT_FUNC,
    TT_VAR,
    TT_RETURN,

    // general punctuation
    TT_SEMICOLON,
    TT_COMMA,
    TT_AT,
    TT_LBRACE,
    TT_RBRACE,

    // expression punctuation
    TT_EQ,

    TT_LPAREN,
    TT_RPAREN,

    // arithmetic operators
    TT_PLUS,
    TT_MINUS,
    TT_STAR,

    // bitwise operators
    TT_BW_AND,
    TT_BW_OR,
    TT_BW_NOT,
    TT_BW_XOR,

    // comparasion operators
    TT_EQEQ,
    TT_NEQ,
    TT_LT,
    TT_GT,
    TT_LE,
    TT_GE,

    // logical operators
    /*TT_AND,
    TT_OR,
    TT_NOT,*/
};

struct Token {
	enum TokenType type;
	char* value;    // not const because of free
    struct Loc begin;
    // struct Loc end;
    struct Token* next;
};

struct MatchTypePair {
    const char* const match;
    const enum TokenType type;
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

/// @brief Frees all token `value`s, and each token itself
void free_tokenlist(struct Token* head);

#endif
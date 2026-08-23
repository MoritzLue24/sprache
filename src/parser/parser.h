#ifndef PARSER_H
#define PARSER_H

#include "parser/ast.h"

struct Arena;
struct TokenList;
struct DiagList;

/// @brief Parses a token list into an abstract syntax tree.
/// @param tkl Tokens from lex(). Must be TK_END terminated, which the
///     lexer guarantees; it is read but never modified.
/// @param dl Diagnostics are appended here. Never read, never cleared.
/// @return The NODE_PROGRAM root, by value.
struct Node parse(
    struct Arena* a, const struct TokenList* tkl, struct DiagList* dl
);

#endif
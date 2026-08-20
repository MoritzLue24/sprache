#ifndef LEXER_H
#define LEXER_H

#include "lexer/tokens.h"
#include "utils/arena.h"

/// @brief Iterates through the `source`code, returns a linked list of tokens
/// @note Tokens need freeing
struct TokenList lex(struct Arena* a, const char* source);

#endif
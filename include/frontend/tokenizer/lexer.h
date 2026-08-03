#ifndef LEXER_H
#define LEXER_H

#include "frontend/tokenizer/tokens.h"


/// @brief Iterates through the `source`code, returns a linked list of tokens
/// @note Tokens need freeing
struct Token* lex(const char* source);

#endif
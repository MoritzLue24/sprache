#ifndef LEXER_H
#define LEXER_H

#include "lexer/tokens.h"

struct Arena;

struct TokenList lex(struct Arena* a, const char* source);

#endif

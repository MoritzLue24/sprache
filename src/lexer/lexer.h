#ifndef LEXER_H
#define LEXER_H

#include "lexer/tokens.h"
#include "utils/arena.h"

struct TokenList lex(struct Arena* a, const char* source);

#endif

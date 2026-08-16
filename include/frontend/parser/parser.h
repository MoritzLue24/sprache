#ifndef PARSER_H
#define PARSER_H

#include "core/error.h"
#include "frontend/tokenizer/tokens.h"
#include "frontend/parser/ast.h"


struct Node* parse(const struct Token* tokens, struct ErrorList* errorlist);

#endif
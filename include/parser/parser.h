#ifndef PARSER_H
#define PARSER_H

#include "core/error.h"
#include "parser/tokens.h"
#include "parser/ast.h"


struct Node* parse(const struct Token* tokens, struct ErrorList* errorlist);

#endif
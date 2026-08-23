#ifndef PARSER_H
#define PARSER_H

#include "parser/ast.h"

struct Arena;
struct TokenList;
struct DiagList;

struct Node parse(
    struct Arena* a, const struct TokenList* tkl, struct DiagList* dl
);

#endif
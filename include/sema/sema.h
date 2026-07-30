#ifndef SEMA_H
#define SEMA_H

#include "core/error.h"
#include "parser/ast.h"
#include "sema/symbols.h"


void check_sema(struct Node* root, struct ErrorList* errorlist, struct SymTable* st);

#endif
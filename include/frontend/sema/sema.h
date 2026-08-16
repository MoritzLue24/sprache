#ifndef SEMA_H
#define SEMA_H

#include "core/error.h"
#include "frontend/parser/ast.h"
#include "frontend/sema/symbols.h"


void check_sema(struct Node* node, struct ErrorList* errorlist, struct SymTable* st);

#endif
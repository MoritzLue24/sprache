#ifndef IRGEN_H
#define IRGEN_H

#include "gen/irinstr.h"
#include "parser/ast.h"
#include "core/error.h"


struct IRInstr* gen_ir(struct Node* root, struct ErrorList* error_list);

#endif
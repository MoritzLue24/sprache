#ifndef IRGEN_H
#define IRGEN_H

#include "backend/ir/irinstr.h"
#include "frontend/parser/ast.h"


struct IRInstr* gen_ir(struct Node* root);

#endif
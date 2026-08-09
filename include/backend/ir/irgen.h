#ifndef IRGEN_H
#define IRGEN_H

#include "backend/ir/irinstr.h"
#include "frontend/parser/ast.h"


struct IRFunc* gen_ir(const struct Node* root);

#endif
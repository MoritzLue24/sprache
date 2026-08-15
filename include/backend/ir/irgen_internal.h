#ifndef IRGEN_INTERNAL_H
#define IRGEN_INTERNAL_H

#include "frontend/parser/ast.h"
#include "backend/ir/irinstr.h"
#include "backend/ir/stack_frame.h"


struct IROperand new_func_op(const char* ident);

struct IROperand new_var_op(struct SFEntry* sf_entry);

struct IROperand new_vreg_op();

struct IROperand new_imm_op(int value);

unsigned int vreg_cost(const struct Node* node);

void use_instrlist(struct IRInstr** head_p);

void unuse_instrlist(struct IRInstr** head_p);

void push_instr(struct IRInstr* instr);

unsigned int vreg_cost(const struct Node* node);

#endif
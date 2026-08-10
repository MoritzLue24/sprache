#include "backend/ir/irgen_internal.h"

#include <assert.h>

#include "utils/xalloc.h"


static unsigned int vreg_cnt = 0;
static struct IRInstr** head;
static struct IRInstr* tail;


struct IROperand new_func_op(const char* ident)
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_FUNC,
        .func.ident = xstrdup(ident)
    };
}

struct IROperand new_var_op(struct SFEntry* sf_entry)
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_VAR,
        .var.sf_entry = sf_entry
    };
}

struct IROperand new_vreg_op()
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_REG,
        .reg.regalloc_done = false,
        .reg.vreg_i = vreg_cnt++
    };
}

struct IROperand new_imm_op(int value)
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_IMM,
        .imm.value = value
    };
}

struct IRInstr* new_instr(enum IROp op, struct IROperand dest, struct IROperand src1, struct IROperand src2)
{
    struct IRInstr* instr = xmalloc(sizeof(struct IRInstr));
    instr->op = op;
    instr->dest = dest;
    instr->src1 = src1;
    instr->src2 = src2;
    instr->next = NULL;
    return instr;
}

// FIXME: is not optimal, atm its just basic leaves-counting
// see https://www.geeksforgeeks.org/compiler-design/labeling-algorithm-in-compiler-design/
unsigned int vreg_cost(const struct Node* node)
{
    switch (node->type) {
    case NODE_VAR:
        return 1;

    case NODE_LITERAL:
        return 1;

    case NODE_BINARY_OP:
        return vreg_cost(node->bin_op.lhs) + vreg_cost(node->bin_op.rhs);

    case NODE_RETURN:
        return vreg_cost(node->ret.expr);

    default:
        assert(0);
    }
}

void use_instrlist(struct IRInstr** head_p)
{
    head = head_p;
    tail = *head_p;
}

void unuse_instrlist(struct IRInstr** head_p)
{
    assert(head_p == head);
    head = NULL;
    tail = NULL;
}

void push_instr(struct IRInstr* instr)
{
    assert(head);
    if (*head == NULL) {
        *head = instr;
        tail = *head;
    }
    else {
        tail->next = instr;
        tail = tail->next;
    }
}
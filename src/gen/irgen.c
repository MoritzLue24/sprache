#include "gen/irgen.h"

#include <assert.h>
#include "utils/xalloc.h"
#include "sema/symbols.h"
#include "core/error.h"
#include "gen/stack_frame.h"


static struct ErrorList* errorlist = NULL;
static struct IRInstr* tail = NULL;
static struct IRInstr* head = NULL;

/// @brief virtual register count
static unsigned int vreg_cnt = 0;

static struct StackFrame* sf = NULL;


static struct IROperand new_var_op(unsigned int sf_offset)
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_VAR,
        .sf_offset = sf_offset
    };
}

static struct IROperand new_vreg_op()
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_VREG,
        .vreg_i = vreg_cnt++
    };
}

static struct IROperand new_imm_op(int value)
{
    return (struct IROperand){
        .none = false,
        .type = OPRND_IMM,
        .imm = value
    };
}

static struct IROperand push(enum IROp op, struct IROperand dest, struct IROperand src1, struct IROperand src2)
{
    if (tail == NULL) {
        tail = xmalloc(sizeof(struct IRInstr));
        head = tail;
    }
    else {
        tail->next = xmalloc(sizeof(struct IRInstr));
        tail = tail->next;
    }
    tail->op = op;
    tail->dest = dest;
    tail->src1 = src1;
    tail->src2 = src2;
    tail->next = NULL;
    return tail->dest;
}

// FIXME: is not optimal, atm its just basic leaves-counting
// see https://www.geeksforgeeks.org/compiler-design/labeling-algorithm-in-compiler-design/
static unsigned int vreg_cost(const struct Node* node)
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

static struct IROperand gen_imm(struct Node* lit)
{
    assert(lit->type == NODE_LITERAL);
    int val = atoi(lit->literal.value);
    return push(IR_IMM, new_vreg_op(), new_imm_op(val), EMPTY_OPRND);
}

static struct IROperand gen_binop(struct Node* binop, struct IROperand src1, struct IROperand src2)
{
    assert(binop->type == NODE_BINARY_OP);
    struct IROperand dest = new_vreg_op();
    switch (binop->bin_op.op) {
        case OP_PLUS:
            return push(IR_ADD, dest, src1, src2);
            break;

        case OP_MINUS:
            return push(IR_SUB, dest, src1, src2);
            break;

        case OP_MUL:
            return push(IR_MUL, dest, src1, src2);

        default:
            assert(0);
    }
}

static struct IROperand gen_instr(struct Node* node)
{
    switch (node->type) {
        case NODE_LITERAL:
        {
            return gen_imm(node);
        }

        case NODE_BINARY_OP:
        {
            unsigned int lhs_cost = vreg_cost(node->bin_op.lhs);
            unsigned int rhs_cost = vreg_cost(node->bin_op.rhs);

            struct IROperand lhs, rhs;
            if (lhs_cost >=rhs_cost) {
                lhs = gen_instr(node->bin_op.lhs);
                rhs = gen_instr(node->bin_op.rhs);
            }
            else {
                rhs = gen_instr(node->bin_op.rhs);
                lhs = gen_instr(node->bin_op.lhs);
            }
            return gen_binop(node, lhs, rhs);
        }

        case NODE_RETURN:
        {
            return push(IR_RETURN, EMPTY_OPRND, gen_instr(node->ret.expr), EMPTY_OPRND);
        }

        case NODE_VAR_DECL:
        {
            unsigned int sf_offset = stackframe_push(sf, node->var_decl.symbol);
            return new_var_op(sf_offset);
        }

        case NODE_VAR_DEF:
        {
            unsigned int sf_offset = stackframe_push(sf, node->var_def.symbol);
            struct IROperand dest = push(IR_STORE_LOCAL, new_var_op(sf_offset), gen_instr(node->var_def.expr), EMPTY_OPRND);
            return dest;
        }

        case NODE_VAR_ASSIGN:
        {
            unsigned int sf_offset;
            if (!get_sf_offset(sf, node->var_assign.symbol, &sf_offset)) {
                assert(0);
            }
            return push(IR_STORE_LOCAL, new_var_op(sf_offset), gen_instr(node->var_assign.expr), EMPTY_OPRND);
        }

        case NODE_VAR:
        {
            unsigned int sf_offset;
            if (!get_sf_offset(sf, node->var.symbol, &sf_offset)) {
                assert(0);
            }
            return push(IR_LOAD_LOCAL, new_vreg_op(), new_var_op(sf_offset), EMPTY_OPRND);
        }

        default:
            assert(0);
    }
}

struct IRInstr* gen_ir(struct Node* root, struct ErrorList* error_list)
{
    errorlist = error_list;
    vreg_cnt = 0;
    tail = head = NULL;

    sf = xmalloc(sizeof(struct StackFrame));

    switch (root->type) {
        // NODE_BLOCK is handled as a function for now
        case NODE_BLOCK:
            push(IR_INIT_SF, EMPTY_OPRND, new_imm_op(0), EMPTY_OPRND);
            struct IRInstr* init_sf = tail; // remember, to set src1 as stack size
            init_stackframe(sf, 10);

            struct Node* cur_node = nodelist_pop_first(&root->block.nodes);
            while (cur_node != NULL) {
                gen_instr(cur_node);
                free_ast(cur_node);
                cur_node = nodelist_pop_first(&root->block.nodes);
            }

            // -1 because it starts at 1
            unsigned int sf_size = get_stacksize(sf);
            init_sf->src1.imm = sf_size;
            push(IR_FREE_SF, EMPTY_OPRND, new_imm_op(sf_size), EMPTY_OPRND);
            break;

         default:
            assert(0);
    }
    return head;
}

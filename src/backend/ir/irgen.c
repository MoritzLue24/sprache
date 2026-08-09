#include "backend/ir/irgen.h"
#include "backend/ir/irgen_internal.h"

#include <assert.h>

#include "utils/xalloc.h"
#include "frontend/sema/symbols.h"
#include "backend/ir/stack_frame.h"


// forward decl
static struct IRFunc* gen_func_def(const struct Node* node);
static struct IROperand gen_instr(const struct Node* node);
static struct IROperand gen_block(const struct Node* node);
static struct IROperand gen_var_def(const struct Node* node);
static struct IROperand gen_return(const struct Node* node);
static struct IROperand gen_assign_expr(const struct Node* node);
static struct IROperand gen_bin_op(const struct Node* node);
static struct IROperand gen_var(const struct Node* node);
static struct IROperand gen_call(const struct Node* node);
static struct IROperand gen_literal(const struct Node* node);


struct IRFunc* gen_ir(const struct Node* node)
{
    assert(node->type == NODE_PROGRAM);
    struct IRFunc* head = NULL;
    struct IRFunc* tail = NULL;

    for (size_t i = 0; i < node->program.items.size; i++) {
        const struct Node* item = node->program.items.data[i];
        assert(item->type == NODE_FUNC_DEF);

        struct IRFunc* fn = gen_func_def(item);
        if (head == NULL) {
            head = fn;
        }
        else {
            tail->next = fn;
        }
        tail = fn;
    }
    return head;
}

static struct IRFunc* gen_func_def(const struct Node* node)
{
    assert(node->type == NODE_FUNC_DEF);
    assert(node->func_def.ident);

    // init result
    struct IRFunc* fn = xmalloc(sizeof(struct IRFunc));
    fn->ident = xstrdup(node->func_def.ident);
    fn->instrs = NULL;
    fn->next = NULL;
    init_stackframe(&fn->sf, fn->ident);
    use_stackframe(&fn->sf);

    // instr: alloc sf
    use_instrlist(&fn->instrs);
    struct IRInstr* alloc_sf_instr = new_instr(
        IR_ALLOC_SF, new_imm_op(0), EMPTY_OPRND, EMPTY_OPRND
    );
    push_instr(alloc_sf_instr);

    // push arguments to the stackframe
    for (size_t i = 0; i < node->func_def.params.size; i++) {
        const struct Node* param_n = node->func_def.params.data[i];
        assert(param_n->type == NODE_PARAM);
        assert(param_n->param.symbol);
        stackframe_push(param_n->param.symbol, true);
    }

    // gen body
    gen_instr(node->func_def.body);

    // calculate stackpointer offsets for arguments
    // arguments are stored by a offset relative to the basepointer + ret_addr 
    resolve_arg_offsets();

    // instr: drop
    // & update stacksize in alloc instr
    alloc_sf_instr->src1 = new_imm_op(fn->sf.s_size);
    push_instr(new_instr(
        IR_DROP_SF,
        EMPTY_OPRND,
        new_imm_op(fn->sf.s_size),
        new_func_op(get_func_ident())
    ));
    unuse_instrlist(&fn->instrs);

    // ret
    unuse_stackframe(&fn->sf);
    return fn;
}

static struct IROperand gen_instr(const struct Node* node)
{
    switch (node->type) {
        case NODE_BLOCK:
            return gen_block(node);
        case NODE_VAR_DECL:
            assert(node->var_decl.target->type == NODE_VAR);
            stackframe_push(node->var_decl.target->var.symbol, false);
            return EMPTY_OPRND;
        case NODE_VAR_DEF:
            return gen_var_def(node);
        case NODE_RETURN:
            return gen_return(node);
        case NODE_ASSIGN_EXPR:
            return gen_assign_expr(node);
        case NODE_BINARY_OP:
            return gen_bin_op(node);
        case NODE_VAR:
            return gen_var(node);
        case NODE_CALL:
            return gen_call(node);
        case NODE_LITERAL:
            return gen_literal(node);
        case NODE_BUILTIN:
        default:
            assert(0);
    }
}

/// @brief writes to the current irlist ctx
static struct IROperand gen_block(const struct Node* node)
{
    assert(node->type == NODE_BLOCK);
    for (size_t i = 0; i < node->block.statements.size; i++) {
        const struct Node* cur = node->block.statements.data[i];
        gen_instr(cur);
    }
    return EMPTY_OPRND;
}

static struct IROperand gen_var_def(const struct Node* node)
{
    assert(node->type == NODE_VAR_DEF);
    assert(node->var_def.target->type == NODE_VAR);

    struct SFEntry* sf_entry = stackframe_push(node->var_decl.target->var.symbol, false);

    struct IROperand expr_op = gen_instr(node->var_def.expr);
    push_instr(new_instr(
        IR_STORE_LOCAL,
        new_var_op(sf_entry), expr_op, EMPTY_OPRND
    ));
    return EMPTY_OPRND;
}

static struct IROperand gen_return(const struct Node* node)
{
    assert(node->type == NODE_RETURN);

    struct IROperand expr_op = gen_instr(node->ret.expr);
    push_instr(new_instr(
        IR_RETURN,
        EMPTY_OPRND, expr_op, new_func_op(get_func_ident())
    ));
    return EMPTY_OPRND;
}

static struct IROperand gen_assign_expr(const struct Node* node)
{
    assert(node->type == NODE_ASSIGN_EXPR);
    assert(node->assign_expr.target->type == NODE_VAR);

    struct SFEntry* entry = stackframe_lookup(node->assign_expr.target->var.symbol);
    assert(entry);

    struct IROperand target_op = new_var_op(entry);
    struct IROperand expr_op = gen_instr(node->assign_expr.expr);
    push_instr(new_instr(
        IR_STORE_LOCAL,
        target_op, expr_op, EMPTY_OPRND
    ));
    return expr_op;
}

static struct IROperand gen_bin_op(const struct Node* node)
{
    assert(node->type == NODE_BINARY_OP);

    unsigned int lhs_cost = vreg_cost(node->bin_op.lhs);
    unsigned int rhs_cost = vreg_cost(node->bin_op.rhs);

    struct IROperand lhs, rhs;
    if (lhs_cost >= rhs_cost) {
        lhs = gen_instr(node->bin_op.lhs);
        rhs = gen_instr(node->bin_op.rhs);
    }
    else {
        rhs = gen_instr(node->bin_op.rhs);
        lhs = gen_instr(node->bin_op.lhs);
    }
    struct IROperand dest = new_vreg_op();

    struct IRInstr* instr;
    switch (node->bin_op.op) {
        case OP_PLUS:
            instr = new_instr(IR_ADD, dest, lhs, rhs);
            break;
        case OP_MINUS:
            instr = new_instr(IR_SUB, dest, lhs, rhs);
            break;
        case OP_MUL:
            instr = new_instr(IR_MUL, dest, lhs, rhs);
            break;
        default:
            assert(0);
    }
    push_instr(instr);
    return instr->dest;
}

static struct IROperand gen_var(const struct Node* node)
{
    assert(node->type == NODE_VAR);

    struct SFEntry* entry = stackframe_lookup(node->var.symbol);
    assert(entry);

    struct IROperand dest = new_vreg_op();
    struct IROperand op = new_var_op(entry);
    push_instr(new_instr(IR_LOAD_LOCAL, dest, op, EMPTY_OPRND));
    return dest;
}

static struct IROperand gen_call(const struct Node* node)
{
    assert(node->type == NODE_CALL);

    for (int i = (int)node->call.args.size - 1; i >= 0; i--) {
        const struct Node* arg_n = node->call.args.data[i];
        struct IROperand src = gen_instr(arg_n);
        push_instr(new_instr(IR_PUSH_ARG, EMPTY_OPRND, src, EMPTY_OPRND));
    }

    struct IROperand dest = new_vreg_op();
    struct IROperand src = new_func_op(node->call.ident);
    push_instr(new_instr(IR_CALL, dest, src, EMPTY_OPRND));

    for (size_t i = 0; i < node->call.args.size; i++) {
        push_instr(new_instr(IR_POP_ARG, EMPTY_OPRND, EMPTY_OPRND, EMPTY_OPRND));
    }
    return dest;
}

static struct IROperand gen_literal(const struct Node* node)
{
    assert(node->type == NODE_LITERAL);

    int val = atoi(node->literal.value);    // FIXME
    struct IROperand dest = new_vreg_op();
    struct IROperand src = new_imm_op(val);

    push_instr(new_instr(IR_IMM, dest, src, EMPTY_OPRND));
    return dest;
}

#include "frontend/sema/sema.h"

#include <assert.h>

#include "utils/xalloc.h"
#include "frontend/sema/builtins.h"
#include "backend/target/avr_target.h"  // FIXME: no dependency


struct ErrorList* errors = NULL;
static struct SymTable* st = NULL;


// helper
static bool is_modifiable_lvalue(struct Node* node)
{
    switch (node->type) {
        case NODE_VAR:
            return true;
        default:
            return false;
    }
}


// forward decl
static void check_node(struct Node* node);
static void check_nodelist(struct NodeList* nl);
static void check_node_func_def(struct Node* node);
static void check_block(struct Node* node, const struct NodeList* param_nl);
static void check_var_decl(struct Node* node);
static void check_var_def(struct Node* node);
static void check_return(struct Node* node);
static void check_assign_expr(struct Node* node);
static void check_binop(struct Node* node);
static void check_var(struct Node* node);
static void check_call(struct Node* node);
static void check_builtin(struct Node* node);


void check_sema(struct Node* node, struct ErrorList* errorlist, struct SymTable* st_dest)
{
    errors = errorlist;
    st = st_dest;

    symtable_enter_scope(st, 10);  // target scope
    target_declare_symbols(st);
    check_node(node);
    symtable_exit_scope(st);
}

static void check_node(struct Node* node)
{
    switch (node->type) {
        case NODE_PROGRAM:
            check_nodelist(&node->program.items);
            break;

        case NODE_FUNC_DEF:
            check_node_func_def(node);
            break;

        case NODE_BLOCK:
            check_block(node, NULL);
            break;

        case NODE_VAR_DECL:
            check_var_decl(node);
            break;

        case NODE_VAR_DEF:
            check_var_def(node);
            break;

        case NODE_RETURN:
            check_return(node);
            break;

        case NODE_ASSIGN_EXPR:
            check_assign_expr(node);
            break;

        case NODE_BINARY_OP:
            check_binop(node);
            break;

        case NODE_VAR:
            check_var(node);
            break;

        case NODE_CALL:
            check_call(node);
            break;

        case NODE_BUILTIN:
            check_builtin(node);
            break;

        case NODE_LITERAL:
            break;

        default:
            assert(0);
    }
}

static void check_nodelist(struct NodeList* nl)
{
    for (size_t i = 0; i < nl->size; i++) {
        check_node(nl->data[i]);
    }
}

static void check_node_func_def(struct Node* node)
{
    assert(node->type == NODE_FUNC_DEF);
    const struct Symbol* existing = symtable_lookup(st, node->func_def.ident);
    if (existing != NULL) {
        add_redeclaration_error(errors, existing, node->begin);
    }
    node->func_def.symbol = symtable_declare_func(st, node->func_def.ident, node->begin, node->func_def.params.size);

    // pass params to declare & check
    check_block(node->func_def.body, &node->func_def.params);
}

static void check_block(struct Node* node, const struct NodeList* param_nl)
{
    assert(node->type == NODE_BLOCK);
    symtable_enter_scope(st, 10);

    if (param_nl != NULL) {
        for (size_t i = 0; i < param_nl->size; i++) {
            struct Node* param_n = param_nl->data[i];
            assert(param_n->type == NODE_PARAM);

            const struct Symbol* existing = symtable_lookup(st, param_n->param.ident);
            if (existing != NULL) {
                add_redeclaration_error(errors, existing, param_n->begin);
            }
            else {
                param_n->param.symbol = symtable_declare(st, SYM_LOCAL, param_n->param.ident, param_n->begin);
            }
        }
    }

    check_nodelist(&node->block.statements);
    symtable_exit_scope(st);
}

static void check_var_decl(struct Node* node)
{
    assert(node->type == NODE_VAR_DECL);
    assert(node->var_decl.target->type == NODE_VAR);

    const struct Symbol* existing = symtable_lookup(st, node->var_decl.target->var.ident);
    if (existing != NULL) {
        add_redeclaration_error(errors, existing, node->begin);
        return;
    }
    node->var_decl.target->var.symbol = symtable_declare(st, SYM_LOCAL, node->var_decl.target->var.ident, node->begin);
}

static void check_var_def(struct Node* node)
{
    assert(node->type == NODE_VAR_DEF);
    assert(node->var_def.target->type == NODE_VAR);

    const struct Symbol* existing = symtable_lookup(st, node->var_def.target->var.ident); 
    if (existing != NULL) {
        add_redeclaration_error(errors, existing, node->begin);
    }
    else {
        node->var_def.target->var.symbol = symtable_declare(st, SYM_LOCAL, node->var_def.target->var.ident, node->begin);
    }
    check_node(node->var_def.expr);
}

static void check_return(struct Node* node)
{
    assert(node->type == NODE_RETURN);
    check_node(node->ret.expr);
}

static void check_assign_expr(struct Node* node)
{
    assert(node->type == NODE_ASSIGN_EXPR);
    check_node(node->assign_expr.target);
    if (!is_modifiable_lvalue(node->assign_expr.target)) {
        add_error(
            errors, ERROR_LVALUE_NOT_MODIFIABLE,
            node->assign_expr.target->begin,
            "Assignment of not-modifiable lvalue"
        );
    }
    check_node(node->assign_expr.expr);
}

static void check_binop(struct Node* node)
{
    assert(node->type == NODE_BINARY_OP);
    check_node(node->bin_op.lhs);
    check_node(node->bin_op.rhs);
}

static void check_var(struct Node* node)
{
    assert(node->type == NODE_VAR);

    const struct Symbol* existing = symtable_lookup(st, node->var.ident);
    if (existing == NULL) {
        add_error(
            errors, ERROR_UNDECLARED, node->begin,
            "Usage of an undeclared symbol '%s'",
            node->var.ident
        );
    }
    node->var.symbol = existing;
}

static void check_call(struct Node* node)
{
    assert(node->type == NODE_CALL);
    const struct Symbol* existing = symtable_lookup(st, node->call.ident);
    if (existing == NULL) {
        add_error(
            errors, ERROR_UNDECLARED, node->begin,
            "Usage of an undeclared symbol '%s'",
            node->call.ident
        );
        return;
    }
    if (existing->type != SYM_FUNC) {
        add_error(
            errors, ERROR_NOT_CALLABLE, node->begin,
            "Symbol '%s' is not a function",
            node->call.ident
        );
        return;
    }
    node->call.symbol = existing;

    if (existing->func.param_count != node->call.args.size) {
        add_error(
            errors, ERROR_INVALID_ARG_SIZE, node->begin,
            "Function '%s' expects %li argument(s), got %li",
            node->call.ident, existing->func.param_count, node->call.args.size
        );
    }
}

static void check_builtin(struct Node* node)
{
    assert(node->type == NODE_BUILTIN);
    const struct BuiltinDef* builtin_def = match_builtin(node->builtin.ident);

    if (builtin_def == NULL) {
        add_error(
            errors, ERROR_INVALID_BUILTIN, node->begin,
            "Builtin does not exist: '%s'",
            node->builtin.ident
        );
        return;
    }
    node->builtin.def = builtin_def;

    if (node->builtin.args.size != builtin_def->param_count) {
        add_error(
            errors, ERROR_INVALID_ARG_SIZE, node->begin,
            "Builtin '%s' expects %li argument(s), got %li",
            node->builtin.ident, builtin_def->param_count, node->builtin.args.size
        );
    }
}

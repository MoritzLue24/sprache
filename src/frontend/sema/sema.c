#include "frontend/sema/sema.h"

#include <assert.h>

#include "utils/xalloc.h"
#include "frontend/sema/builtins.h"
#include "backend/target/avr_target.h"  // FIXME: no dependency


static struct ErrorList* errors = NULL;
static struct SymTable* st = NULL;




// forward decl
static void check_node(struct Node* node);
static void check_node_func_def(struct Node* node);
static void check_block(struct Node* node, struct Node* param_nl);
static void check_var_decl(struct Node* node);
static void check_var_def(struct Node* node);
static void check_return(struct Node* node);
static void check_assign_expr(struct Node* node);
static void check_unary_op(struct Node* node);
static void check_binop(struct Node* node);
static void check_var(struct Node* node);
static void check_assign_target(struct Node* node);
static void check_call(struct Node* node);
static void check_builtin(struct Node* node);


void check_sema(struct Node* node, struct ErrorList* errorlist, struct SymTable* st_dest)
{
    errors = errorlist;
    st = st_dest;
    check_node(node);

    const struct Symbol* main_sym = symtable_lookup(st, "main");
    if (main_sym == NULL || main_sym->type != SYM_FUNC) {
        add_error(errors, ERROR_MAIN_MISSING, EMPTY_LOC, "'main' function is missing");
    }
}

static void check_node(struct Node* node)
{
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_PROGRAM:
            check_node(node->program.items_head);
            //check_nodelist(&node->program.items_head);
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

        case NODE_UNARY_OP:
            check_unary_op(node);
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

        case NODE_INVALID:
            break;

        default:
            assert(0);
    }

    if (node->next != NULL) {
        check_node(node->next);
    }
}

static void check_node_func_def(struct Node* node)
{
    assert(node->type == NODE_FUNC_DEF);
    const struct Symbol* existing = symtable_lookup(st, node->func_def.ident);
    if (existing != NULL) {
        add_redeclaration_error(errors, existing, node->begin);
    }
    else {
        node->func_def.symbol = symtable_declare_func(
            st,
            node->func_def.ident,
            node->begin,
            nodelist_size(node->func_def.params_head)
        );
    }

    // pass params to declare & check
    check_block(node->func_def.body, node->func_def.params_head);
}

static void check_block(struct Node* node, struct Node* param_nl)
{
    if (node->type == NODE_INVALID) {
        return;
    }
    assert(node->type == NODE_BLOCK);
    symtable_enter_scope(st, 10);

    for (; param_nl != NULL; param_nl = param_nl->next) {
        if (param_nl->type == NODE_INVALID) {
            continue;
        }
        assert(param_nl->type == NODE_PARAM);

        const struct Symbol* existing = symtable_lookup(st, param_nl->param.ident);
        if (existing != NULL) {
            add_redeclaration_error(errors, existing, param_nl->begin);
        }
        else {
            param_nl->param.symbol = symtable_declare(
                st,
                SYM_LOCAL,
                param_nl->param.ident,
                param_nl->begin
            );
        }
    }

    check_node(node->block.statements_head);
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
    check_assign_target(node->assign_expr.target);
    check_node(node->assign_expr.expr);
}

static void check_unary_op(struct Node* node)
{
    assert(node->type == NODE_UNARY_OP);
    check_node(node->unary_op.factor);
}

static void check_binop(struct Node* node)
{
    assert(node->type == NODE_BINARY_OP);
    check_node(node->bin_op.lhs);
    check_node(node->bin_op.rhs);
}

/// @brief Resolves & annotates the symbol of a NODE_VAR, reports ERROR_UNDECLARED
/// if there is none. Says nothing about whether the symbol may be used the way
/// it is used here - that is up to the caller (`check_var` / `check_assign_target`).
/// @return the resolved symbol, NULL if the ident is undeclared
static const struct Symbol* resolve_var(struct Node* node)
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
    return existing;
}

/// @brief Checks a NODE_VAR that is READ (used as a value).
///
/// Only locals live in a stackframe, so only they can be read as a value.
/// The backend (`gen_var`) relies on that: it looks the symbol up in the
/// stackframe and asserts it is there.
static void check_var(struct Node* node)
{
    const struct Symbol* sym = resolve_var(node);
    if (sym == NULL) {
        return;     // already reported as undeclared
    }

    // no `default`: a new SymbolType should make this switch fail to compile
    switch (sym->type) {
        case SYM_LOCAL:
            break;

        case SYM_FUNC:
            add_error(
                errors, ERROR_NOT_A_VALUE, node->begin,
                "Function '%s' can only be called, not used as a value",
                node->var.ident
            );
            break;

        case SYM_TARGET:
            // TODO: needs an IR op that reads the io register (`in rX, PINA`)
            add_error(
                errors, ERROR_NOT_A_VALUE, node->begin,
                "Target constant '%s' cannot be read yet",
                node->var.ident
            );
            break;
    }
}

/// @brief Checks the target (lvalue) of an assignment.
///
/// Assignability depends on the SYMBOL, not on the node type: NODE_VAR is just
/// "some ident used as an expression" and also covers functions and target
/// constants, neither of which has a stackframe slot to store into.
static void check_assign_target(struct Node* node)
{
    if (node->type != NODE_VAR) {
        check_node(node);
        add_error(
            errors, ERROR_LVALUE_NOT_MODIFIABLE, node->begin,
            "Assignment target is not a variable"
        );
        return;
    }

    const struct Symbol* sym = resolve_var(node);
    if (sym == NULL) {
        return;     // already reported as undeclared
    }

    switch (sym->type) {
        case SYM_LOCAL:
            break;

        case SYM_FUNC:
            add_error(
                errors, ERROR_LVALUE_NOT_MODIFIABLE, node->begin,
                "Cannot assign to function '%s'",
                node->var.ident
            );
            break;

        case SYM_TARGET:
            add_error(
                errors, ERROR_LVALUE_NOT_MODIFIABLE, node->begin,
                "'%s' is a read-only target constant",
                node->var.ident
            );
            break;
    }
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

    size_t args_size = nodelist_size(node->call.args_head);
    if (existing->func.param_count != args_size) {
        add_error(
            errors, ERROR_INVALID_ARG_SIZE, node->begin,
            "Function '%s' expects %li argument(s), got %li",
            node->call.ident, existing->func.param_count, args_size
        );
    }

    check_node(node->call.args_head);
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

    size_t args_size = nodelist_size(node->builtin.args_head);
    if (args_size != builtin_def->param_count) {
        add_error(
            errors, ERROR_INVALID_ARG_SIZE, node->begin,
            "Builtin '%s' expects %li argument(s), got %li",
            node->builtin.ident, builtin_def->param_count, args_size
        );
    }

    check_node(node->builtin.args_head);
}

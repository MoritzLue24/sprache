#include "sema/sema.h"

#include <assert.h>

#include "utils/xalloc.h"
#include "sema/builtins.h"
#include "target/avr_target.h"


struct ErrorList* errors = NULL;
static struct SymTable* st = NULL;


// forward decl
static void check_node(struct Node* node);

static void check_block(struct Node* node)
{
    symtable_enter_scope(st, 10);

    // do not consume nodes
    for (size_t i = 0; i < node->block.nodes.size; i++) {
        struct Node* cur = nodelist_get(&node->block.nodes, i);
        check_node(cur);
    }

    symtable_exit_scope(st);
}

static void check_binop(struct Node* node)
{
    assert(node->type == NODE_BINARY_OP);
    check_node(node->bin_op.lhs);
    check_node(node->bin_op.rhs);
}

static void check_var_decl(struct Node* node)
{
    assert(node->type == NODE_VAR_DECL);

    struct Symbol* existing = symtable_lookup(st, node->var_decl.ident); 
    if (existing != NULL) {
        switch (existing->type)
        {
            case SYM_LOCAL:
                add_error(
                    errors, ERROR_REDECLARATION, node->begin,
                    "Symbol '%s' already declared at line %i, column %i",
                    existing->ident, existing->loc_decl.ln, existing->loc_decl.col
                );
                break;

            case SYM_TARGET:
                add_error(
                    errors, ERROR_REDECLARATION, node->begin,
                    "Symbol '%s' is a target constant",
                    existing->ident, existing->loc_decl.ln, existing->loc_decl.col
                );
                break;

            default:
                assert(0);
        }
        return;
    }
    node->var_decl.symbol = symtable_declare(st, SYM_LOCAL, node->var_decl.ident, node->begin);
    xfree((void**)&node->var_decl.ident);
}

static void check_var_def(struct Node* node)
{
    assert(node->type == NODE_VAR_DEF);

    struct Symbol* existing = symtable_lookup(st, node->var_def.ident); 
    if (existing != NULL) {
        switch (existing->type)
        {
            case SYM_LOCAL:
                add_error(
                    errors, ERROR_REDECLARATION, node->begin,
                    "Symbol '%s' already declared at line %i, column %i",
                    existing->ident, existing->loc_decl.ln, existing->loc_decl.col
                );
                break;

            case SYM_TARGET:
                add_error(
                    errors, ERROR_REDECLARATION, node->begin,
                    "Symbol '%s' is a target constant",
                    existing->ident, existing->loc_decl.ln, existing->loc_decl.col
                );
                break;

            default:
                assert(0);
        }
        return;
    }
    else {
        node->var_def.symbol = symtable_declare(st, SYM_LOCAL, node->var_def.ident, node->begin);
        xfree((void**)&node->var_def.ident);
    }
    check_node(node->var_def.expr);
}

static void check_var_assign(struct Node* node)
{
    assert(node->type == NODE_VAR_ASSIGN);

    struct Symbol* existing = symtable_lookup(st, node->var_assign.ident);
    if (existing == NULL) {
        add_error(
            errors, ERROR_UNDECLARED, node->begin,
            "Assignment of an undeclared symbol '%s'",
            node->var_assign.ident
        );
    }
    node->var_assign.symbol = existing;
    xfree((void**)&node->var_assign.ident);
    check_node(node->var_assign.expr);
}

static void check_var(struct Node* node)
{
    assert(node->type == NODE_VAR);

    struct Symbol* existing = symtable_lookup(st, node->var.ident);
    if (existing == NULL) {
        add_error(
            errors, ERROR_UNDECLARED, node->begin,
            "Usage of an undeclared symbol '%s'",
            node->var_assign.ident
        );
    }
    node->var.symbol = existing;
    xfree((void**)&node->var.ident);
}

static void check_return(struct Node* node)
{
    assert(node->type == NODE_RETURN);
    check_node(node->ret.expr);
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
    // ident no longer needed for identification
    xfree((void**)&node->builtin.ident);
    node->builtin.def = builtin_def;

    if (node->builtin.args.size != builtin_def->arg_count) {
        add_error(
            errors, ERROR_INVALID_ARG_SIZE, node->begin,
            "Builtin '%s' expects %i arguments, got %i",
            node->builtin.ident, builtin_def->arg_count, node->builtin.args.size
        );
    }
}

static void check_node(struct Node* node)
{
    switch (node->type) {
        case NODE_BLOCK:
            check_block(node);
            break;

        case NODE_LITERAL:
            // Literal always valid on its own.
            // Type checking not required yet,
            // size checking (1 byte) is a target limitation
            break;

        case NODE_BINARY_OP:
            check_binop(node);
            break;

        case NODE_VAR_DECL:
            check_var_decl(node);
            break;

        case NODE_VAR_DEF:
            check_var_def(node);
            break;

        case NODE_VAR_ASSIGN:
            check_var_assign(node);
            break;

        case NODE_VAR:
            check_var(node);
            break;

        case NODE_RETURN:
            check_return(node);
            break;

        case NODE_BUILTIN:
            check_builtin(node);
            break;

        default:
            assert(0);
    }
}

void check_sema(struct Node* root, struct ErrorList* errorlist, struct SymTable* st_dest)
{
    errors = errorlist;
    st = st_dest;

    init_symtable(st, 10);

    symtable_enter_scope(st, 10);  // target scope
    target_declare_symbols(st);
    check_node(root);
    symtable_exit_scope(st);
}
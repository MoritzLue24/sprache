#include "diag/diag_internal.h"
#include "utils/darray.h"
#include "parser/ast.h"
#include "sema/sema.h"
#include "sema/symbols.h"
#include <assert.h>

struct Sema {
    struct Arena* a;
    struct SymTable st;
    /// @brief Current function, NULL at top-level
    const struct Node* cur_func;
    struct DiagList* dl;
};

/// @brief Declares all type symbols to the symtable in the current scope.
/// @note If a symbol that matches with a type-symbol exists, assert.
static void declare_types(struct Sema* s);
// @claude-begin
/// @brief Declares all top-level function definitions,
/// without checking the bodies.
/// @note Annotates symbol & type for the func_def, and types for parameters
/// and every NODE_TYPE of the signature.
static void declare_func_defs(struct Sema* s, struct NodeList toplevel_nl);

/// @brief Resolves the type a NODE_TYPE denotes, and annotates it.
/// @param is_ret_type whether 'none' may be used
/// @returns the resolved type, or TYPE_INVALID (reported) if the name is not
/// a type or the type cannot be used here.
static enum TypeKind resolve_type(
    struct Sema* s, struct Node* n_type, bool is_ret_type
);
/// @brief Declares a variable to the current scope.
/// @returns the declared symbol, NULL (reported) on redeclaration.
static struct Symbol* declare_var(
    struct Sema* s, const char* name, struct SourceLoc loc, enum TypeKind type
);

static void check_func_def(struct Sema* s, struct Node* n);
static void check_statement(struct Sema* s, struct Node* n);
static void check_block(struct Sema* s, struct Node* n);
static void check_var_decl(struct Sema* s, struct Node* n);
static void check_var_def(struct Sema* s, struct Node* n);
static void check_return(struct Sema* s, struct Node* n);
/// @brief iterates through all child nodes (if NODE_BLOCK),
/// til a return is found.
/// @param out_ret_node the return node, set by callee
/// @returns True if a return statement has been found.
static bool find_stmt_return(const struct Node* n, struct Node* out_ret_node);

/// @brief Checks an expression against the expected type, and annotates the
/// whole subtree. The only place where two types are compared.
/// @param expected_type TYPE_INVALID if the context imposes no type
/// @returns the type of the expression, or TYPE_INVALID if it has no usable
/// type. A diagnostic was then already reported, so callers stay silent.
static enum TypeKind check_expr(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
);
/// @note Without an expected type, the operand that has a type of its own is
/// checked first, and its type becomes the expectation for the other one.
static enum TypeKind check_binary(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
);
/// @brief Checks an OP_ASSIGN binary. Its type is the type of the target.
/// @note The target is checked before it is tested for being assignable, so
/// an undeclared name is reported as such.
static enum TypeKind check_assign(struct Sema* s, struct Node* n);
/// @note Unary '-' asks an operand without a type of its own for int8, if
/// the context imposes no type.
static enum TypeKind check_unary(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
);
/// @brief Like check_expr, but rejects an operand of type 'none'.
/// @param n_op the operator node the operand belongs to
static enum TypeKind check_operand(
    struct Sema* s, const struct Node* n_op, struct Node* n,
    enum TypeKind expected_type
);
/// @note A literal adopts the expected type, and falls back to uint8.
static enum TypeKind check_literal(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
);
/// @note Sets n->symbol even if the symbol found is not a variable.
static enum TypeKind check_ident(struct Sema* s, struct Node* n);
static enum TypeKind check_call(struct Sema* s, struct Node* n);
/// @note Builtins are not in the symtable, their symbol is synthesised.
static enum TypeKind check_builtin(struct Sema* s, struct Node* n);
/// @brief Checks every argument against the parameter types of sym.
/// @param sym NULL if the callee could not be resolved. The arguments are
/// then checked without an expected type.
static void check_args(
    struct Sema* s, const struct Node* n_callee, const struct NodeList* args,
    const struct Symbol* sym
);

/// @returns true if the expression has no type of its own, i.e. it is built
/// from literals only.
static bool expr_is_untyped(const struct Node* n);
/// @note The one place to extend once dereferencing, indexing or constant
/// variables exist.
static bool expr_is_modifiable_lvalue(const struct Node* n);

void sema_check(struct Arena* a, struct Node* n_program, struct DiagList* dl)
{
    assert(n_program->kind == NODE_PROGRAM);
    struct Sema sema = { .a = a, .cur_func = NULL, .dl = dl };
    symtable_init(&sema.st);
    symtable_enter_scope(sema.a, &sema.st);

    declare_types(&sema);
    declare_func_defs(&sema, n_program->program.nl);
    for (size_t i = 0; i < n_program->program.nl.count; i++) {
        check_func_def(&sema, n_program->program.nl.items[i]);
    }
    symtable_exit_scope(&sema.st);
}
// @claude-end

static void declare_types(struct Sema* s)
{
    for (size_t i = 0; i < types_count; i++) {
        struct Symbol type_sym = (struct Symbol){
            .kind = SYM_TYPE,
            .name = type_kind_spelling(types[i]),
            .type = types[i],
            .loc_decl = SOURCE_LOC_NULL,
            .paramlist = (struct ParamTypeList){ },
            .builtin_kind = BUILTIN_INVALID,
        };
        const struct Symbol* existing = symtable_declare(
            s->a, &s->st, type_sym
        );
        assert(existing);
    }
}

// @claude-begin
static void declare_func_defs(struct Sema* s, struct NodeList toplevel_nl)
{
    for (size_t i = 0; i < toplevel_nl.count; i++) {
        struct Node* n_func = toplevel_nl.items[i];
        assert(n_func->kind == NODE_FUNC_DEF);

        struct Symbol func_sym = (struct Symbol){
            .kind = SYM_FUNC,
            .name = n_func->value,
            .type = resolve_type(s, n_func->func_def.type, true),
            .loc_decl = n_func->loc,
            .paramlist = (struct ParamTypeList){ },
            .builtin_kind = BUILTIN_INVALID
        };
        DARRAY_INIT(s->a, &func_sym.paramlist, PARAMTYPELIST_INIT_CAPACITY);
        for (size_t j = 0; j < n_func->func_def.params.count; j++) {
            struct Node* n_param = n_func->func_def.params.items[j];
            assert(n_param->kind == NODE_PARAM);

            n_param->type = resolve_type(s, n_param->param.type, false);
            DARRAY_ADD(s->a, &func_sym.paramlist, n_param->type);
        }

        // ANNOTATIONS
        n_func->type = func_sym.type;
        n_func->symbol = symtable_declare(s->a, &s->st, func_sym);
        if (n_func->symbol == NULL) {
            diag_add(
                s->a, s->dl, DIAG_REDECLARATION, n_func->loc, n_func->value
            );
        }
    }
}

static enum TypeKind resolve_type(
    struct Sema* s, struct Node* n_type, bool is_ret_type
) {
    assert(n_type->kind == NODE_TYPE);

    const struct Symbol* sym = symtable_lookup(&s->st, n_type->value);
    if (sym == NULL || sym->kind != SYM_TYPE) {
        diag_add(s->a, s->dl, DIAG_INVALID_TYPE, n_type->loc, n_type->value);
        return TYPE_INVALID;
    }
    // the node denotes the type, even where it cannot be used
    n_type->type = sym->type;
    if (!is_ret_type && sym->type == TYPE_NONE) {
        diag_add(
            s->a, s->dl, DIAG_INVALID_TYPE_USAGE, n_type->loc, n_type->value
        );
        return TYPE_INVALID;
    }
    return sym->type;
}

static struct Symbol* declare_var(
    struct Sema* s, const char* name, struct SourceLoc loc, enum TypeKind type
) {
    struct Symbol* sym = symtable_declare(s->a, &s->st, (struct Symbol){
        .kind = SYM_VAR,
        .name = name,
        .type = type,
        .loc_decl = loc,
        .paramlist = (struct ParamTypeList){ },
        .builtin_kind = BUILTIN_INVALID
    });
    if (sym == NULL) {
        diag_add(s->a, s->dl, DIAG_REDECLARATION, loc, name);
    }
    return sym;
}

static void check_func_def(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_FUNC_DEF);
    // symbol & type already set by 'declare_func_defs'

    s->cur_func = n;
    symtable_enter_scope(s->a, &s->st);
    for (size_t i = 0; i < n->func_def.params.count; i++) {
        struct Node* n_param = n->func_def.params.items[i];
        assert(n_param->kind == NODE_PARAM);

        // type already set by declare_func_defs
        n_param->symbol = declare_var(
            s, n_param->value, n_param->loc, n_param->type
        );
    }

    // not check_block: parameters share their scope with the body's locals
    assert(n->func_def.body->kind == NODE_BLOCK);
    for (size_t i = 0; i < n->func_def.body->block.nl.count; i++) {
        check_statement(s, n->func_def.body->block.nl.items[i]);
    }

    struct Node ret_node;

    if (!find_stmt_return(n->func_def.body, &ret_node)) {
        if (n->type != TYPE_NONE) {
            diag_add(s->a, s->dl, DIAG_MISSING_RETURN, n->loc, n->value);
        }
    }
    else if (ret_node.ret.expr == NULL && n->type != TYPE_NONE) {
        diag_add(
            s->a, s->dl, DIAG_TYPE_MISMATCH, ret_node.loc,
            type_kind_spelling(n->type), 
            type_kind_spelling(TYPE_NONE)
        );
    }

    symtable_exit_scope(&s->st);
    s->cur_func = NULL;
}

static void check_statement(struct Sema* s, struct Node* n)
{
    switch (n->kind) {
        case NODE_BLOCK:
            check_block(s, n);
            break;
        case NODE_VAR_DECL:
            check_var_decl(s, n);
            break;
        case NODE_VAR_DEF:
            check_var_def(s, n);
            break;
        case NODE_RETURN:
            check_return(s, n);
            break;
        default:
            // expression statement, its value is discarded
            check_expr(s, n, TYPE_INVALID);
            break;
    }
}

static void check_block(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_BLOCK);

    symtable_enter_scope(s->a, &s->st);
    for (size_t i = 0; i < n->block.nl.count; i++) {
        check_statement(s, n->block.nl.items[i]);
    }
    symtable_exit_scope(&s->st);
}

static void check_var_decl(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_VAR_DECL);

    n->type = resolve_type(s, n->var_decl.type, false);
    n->symbol = declare_var(s, n->value, n->loc, n->type);
}

static void check_var_def(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_VAR_DEF);

    n->type = resolve_type(s, n->var_def.type, false);
    // before declaring, so 'var a: uint8 = a;' cannot read itself
    check_expr(s, n->var_def.init, n->type);
    n->symbol = declare_var(s, n->value, n->loc, n->type);
}

static void check_return(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_RETURN);
    // not all return statements must return something
    if (n->ret.expr != NULL) {
        check_expr(s, n->ret.expr, s->cur_func->type);
    }
}

static bool find_stmt_return(const struct Node* n, struct Node* out_ret_node)
{
    switch (n->kind) {
        case NODE_RETURN:
            *out_ret_node = *n;
            return true;

        case NODE_BLOCK:
            for (size_t i = 0; i < n->block.nl.count; i++) {
                if (find_stmt_return(n->block.nl.items[i], out_ret_node)) {
                    return true;
                }
            }
            return false;

        default: return false;
    }
}

static enum TypeKind check_expr(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
) {
    enum TypeKind type = TYPE_INVALID;

    switch (n->kind) {
        case NODE_BINARY:
            type = check_binary(s, n, expected_type);
            break;
        case NODE_UNARY:
            type = check_unary(s, n, expected_type);
            break;
        case NODE_LITERAL:
            type = check_literal(s, n, expected_type);
            break;
        case NODE_IDENT:
            type = check_ident(s, n);
            break;
        case NODE_CALL:
            type = check_call(s, n);
            break;
        case NODE_BUILTIN:
            type = check_builtin(s, n);
            break;
        default: assert(false && "not an expression");
    }
    n->type = type;
    if (type == TYPE_INVALID || expected_type == TYPE_INVALID) {
        return type;
    }
    if (type != expected_type) {
        diag_add(
            s->a, s->dl, DIAG_TYPE_MISMATCH, n->loc,
            type_kind_spelling(expected_type), type_kind_spelling(type)
        );
        return TYPE_INVALID;
    }
    return type;
}

static enum TypeKind check_binary(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
) {
    assert(n->kind == NODE_BINARY);
    if (n->op == OP_ASSIGN) return check_assign(s, n);

    struct Node* n_first = n->binary.lhs;
    struct Node* n_second = n->binary.rhs;
    if (expected_type == TYPE_INVALID && expr_is_untyped(n_first)) {
        n_first = n->binary.rhs;
        n_second = n->binary.lhs;
    }

    enum TypeKind type_first = check_operand(s, n, n_first, expected_type);
    // an invalid first operand leaves the second one without expectation
    enum TypeKind type_second = check_operand(s, n, n_second, type_first);
    if (type_first == TYPE_INVALID || type_second == TYPE_INVALID) {
        return TYPE_INVALID;
    }
    return type_first;
}

static enum TypeKind check_assign(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_BINARY && n->op == OP_ASSIGN);
    struct Node* n_target = n->binary.lhs;

    // the target has a type of its own and adopts nothing
    enum TypeKind type_target = check_expr(s, n_target, TYPE_INVALID);
    // TYPE_INVALID: already reported, or a variable of unknown type
    bool assignable = true;
    if (
        type_target != TYPE_INVALID && !expr_is_modifiable_lvalue(n_target)
    ) {
        diag_add(s->a, s->dl, DIAG_NOT_ASSIGNABLE, n_target->loc);
        assignable = false;
    }

    enum TypeKind type_value = check_expr(
        s, n->binary.rhs, assignable ? type_target : TYPE_INVALID
    );
    if (
        !assignable || type_target == TYPE_INVALID
        || type_value == TYPE_INVALID
    ) {
        return TYPE_INVALID;
    }
    return type_target;
}

static enum TypeKind check_unary(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
) {
    assert(n->kind == NODE_UNARY);
    struct Node* n_operand = n->unary.operand;

    enum TypeKind type_operand = expected_type;
    if (
        n->op == OP_NEG
        && expected_type == TYPE_INVALID
        && expr_is_untyped(n_operand)
    ) {
        type_operand = TYPE_INT8;
    }

    type_operand = check_operand(s, n, n_operand, type_operand);
    if (type_operand == TYPE_INVALID) {
        return TYPE_INVALID;
    }
    if (n->op == OP_NEG && !type_is_signed(type_operand)) {
        diag_add(
            s->a, s->dl, DIAG_ILLEGAL_OPERATION_ON_TYPE, n->loc,
            type_kind_spelling(type_operand)
        );
        return TYPE_INVALID;
    }
    return type_operand;
}

static enum TypeKind check_operand(
    struct Sema* s, const struct Node* n_op, struct Node* n,
    enum TypeKind expected_type
) {
    enum TypeKind type = check_expr(s, n, expected_type);
    if (type == TYPE_NONE) {
        diag_add(
            s->a, s->dl, DIAG_ILLEGAL_OPERATION_ON_TYPE, n_op->loc,
            type_kind_spelling(type)
        );
        return TYPE_INVALID;
    }
    return type;
}

static enum TypeKind check_literal(
    struct Sema* s, struct Node* n, enum TypeKind expected_type
) {
    assert(n->kind == NODE_LITERAL);

    // 'none' cannot hold a literal, check_expr reports the mismatch instead
    enum TypeKind type = type_size(expected_type) > 0
        ? expected_type
        : TYPE_UINT8;
    if (!type_fits(type, n->value)) {
        diag_add(
            s->a, s->dl, DIAG_LITERAL_OUT_OF_RANGE, n->loc, n->value,
            type_kind_spelling(type)
        );
        return TYPE_INVALID;
    }
    return type;
}

static enum TypeKind check_ident(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_IDENT);

    n->symbol = symtable_lookup(&s->st, n->value);
    if (n->symbol == NULL) {
        diag_add(s->a, s->dl, DIAG_UNDECLARED, n->loc, n->value);
        return TYPE_INVALID;
    }
    if (n->symbol->kind != SYM_VAR) {
        diag_add(s->a, s->dl, DIAG_NOT_A_VARIABLE, n->loc, n->value);
        return TYPE_INVALID;
    }
    // TYPE_INVALID here was already reported at the declaration
    return n->symbol->type;
}

static enum TypeKind check_call(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_CALL);

    n->symbol = symtable_lookup(&s->st, n->value);
    if (n->symbol == NULL) {
        diag_add(s->a, s->dl, DIAG_UNDECLARED, n->loc, n->value);
        check_args(s, n, &n->call.args, NULL);
        return TYPE_INVALID;
    }
    if (n->symbol->kind != SYM_FUNC) {
        diag_add(s->a, s->dl, DIAG_NOT_A_FUNCTION, n->loc, n->value);
        check_args(s, n, &n->call.args, NULL);
        return TYPE_INVALID;
    }
    check_args(s, n, &n->call.args, n->symbol);
    // the type of the call is known, even if an argument is broken
    return n->symbol->type;
}

static enum TypeKind check_builtin(struct Sema* s, struct Node* n)
{
    assert(n->kind == NODE_BUILTIN);

    enum BuiltinKind kind = builtin_kind_from_spelling(n->value);
    if (kind == BUILTIN_INVALID) {
        diag_add(s->a, s->dl, DIAG_UNKNOWN_BUILTIN, n->loc, n->value);
        check_args(s, n, &n->builtin.args, NULL);
        return TYPE_INVALID;
    }

    n->symbol = ARENA_CALLOC(s->a, struct Symbol);
    *n->symbol = (struct Symbol){
        .kind = SYM_BUILTIN,
        .name = n->value,
        .type = builtin_kind_type(kind),
        .loc_decl = SOURCE_LOC_NULL,
        .paramlist = (struct ParamTypeList){ },
        .builtin_kind = kind
    };
    DARRAY_INIT(s->a, &n->symbol->paramlist, PARAMTYPELIST_INIT_CAPACITY);
    for (size_t i = 0; i < builtin_kind_argc(kind); i++) {
        DARRAY_ADD(
            s->a, &n->symbol->paramlist, builtin_kind_arg_type_at(kind, i)
        );
    }

    check_args(s, n, &n->builtin.args, n->symbol);
    return n->symbol->type;
}

static void check_args(
    struct Sema* s, const struct Node* n_callee, const struct NodeList* args,
    const struct Symbol* sym
) {
    if (sym != NULL && args->count != sym->paramlist.count) {
        diag_add(
            s->a, s->dl, DIAG_ARG_COUNT_MISMATCH, n_callee->loc,
            n_callee->value, sym->paramlist.count, args->count
        );
    }
    for (size_t i = 0; i < args->count; i++) {
        enum TypeKind expected_type = TYPE_INVALID;
        if (sym != NULL && i < sym->paramlist.count) {
            expected_type = sym->paramlist.items[i];
        }
        check_expr(s, args->items[i], expected_type);
    }
}

static bool expr_is_untyped(const struct Node* n)
{
    switch (n->kind) {
        case NODE_LITERAL: return true;
        case NODE_UNARY: return expr_is_untyped(n->unary.operand);
        case NODE_BINARY:
            return n->op != OP_ASSIGN
                && expr_is_untyped(n->binary.lhs)
                && expr_is_untyped(n->binary.rhs);
        default: return false;
    }
}

static bool expr_is_modifiable_lvalue(const struct Node* n)
{
    return n->kind == NODE_IDENT
        && n->symbol != NULL
        && n->symbol->kind == SYM_VAR;
}
// @claude-end

// @claude-begin
// Valid inputs only: every program here has to pass sema without a single
// diagnostic, and with 'type' and 'symbol' annotated as docs/sema.md says.
// Broken inputs live in "sema_error_tests.c".

#include "tst.h"
#include "unit/sema/sema_util.h"

static void sema_empty_program()
{
    struct Parsed p;
    checked_init(&p, "");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    parsed_free(&p);
}

static void sema_annotates_function_signature()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: uint8, b: int8) -> int8 { return b; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ(TYPE_INT8, fn->type);
    TST_ASSERT_EQ(TYPE_INT8, some(fn->func_def.type)->type);
    TST_ASSERT(fn->symbol != NULL);
    if (fn->symbol != NULL) {
        TST_ASSERT_EQ(SYM_FUNC, fn->symbol->kind);
        TST_ASSERT_EQ(TYPE_INT8, fn->symbol->type);
        TST_ASSERT_EQ((size_t)2, fn->symbol->paramlist.count);
        TST_ASSERT_EQ(TYPE_UINT8, fn->symbol->paramlist.items[0]);
        TST_ASSERT_EQ(TYPE_INT8, fn->symbol->paramlist.items[1]);
    }

    const struct Node* a = at(&fn->func_def.params, 0);
    TST_ASSERT_EQ(TYPE_UINT8, a->type);
    TST_ASSERT_EQ(TYPE_UINT8, some(a->param.type)->type);
    TST_ASSERT(a->symbol != NULL && a->symbol->kind == SYM_VAR);

    parsed_free(&p);
}

static void sema_ident_resolves_to_its_declaration()
{
    struct Parsed p;
    checked_init(
        &p, "fn f(a: uint8) -> uint8 { var b: uint8 = a; return b; }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    const struct Node* param = at(&fn->func_def.params, 0);
    const struct Node* def = fn_stmt(&p.root, 0, 0);
    const struct Node* ret = fn_stmt(&p.root, 0, 1);

    // the very same symbol, not an equal copy
    TST_ASSERT(some(def->var_def.init)->symbol == param->symbol);
    TST_ASSERT(some(ret->ret.expr)->symbol == def->symbol);
    TST_ASSERT_EQ(TYPE_UINT8, some(ret->ret.expr)->type);

    parsed_free(&p);
}

static void sema_call_resolves_function_defined_later()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn main() -> uint8 { return add(1, 2); }"
        "fn add(a: uint8, b: uint8) -> uint8 { return a + b; }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* call = some(fn_stmt(&p.root, 0, 0)->ret.expr);
    TST_ASSERT_EQ(NODE_CALL, call->kind);
    TST_ASSERT(call->symbol == at(&p.root.program.nl, 1)->symbol);
    TST_ASSERT_EQ(TYPE_UINT8, call->type);
    TST_ASSERT_EQ(TYPE_UINT8, at(&call->call.args, 0)->type);

    parsed_free(&p);
}

static void sema_literal_adopts_expected_type()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> int8 { return 5; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(TYPE_INT8, some(fn_stmt(&p.root, 0, 0)->ret.expr)->type);

    parsed_free(&p);
}

static void sema_literal_without_context_is_uint8()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { 5; return 0; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(TYPE_UINT8, fn_stmt(&p.root, 0, 0)->type);

    parsed_free(&p);
}

static void sema_literal_adopts_type_of_other_operand()
{
    struct Parsed p;
    // no context: 'a' is checked first although it is the right operand
    checked_init(&p, "fn f(a: int8) -> uint8 { 1 + a; return 0; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* sum = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(TYPE_INT8, sum->type);
    TST_ASSERT_EQ(TYPE_INT8, some(sum->binary.lhs)->type);

    parsed_free(&p);
}

static void sema_negated_literal_without_context_is_int8()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { -1; return 0; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* neg = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(TYPE_INT8, neg->type);
    TST_ASSERT_EQ(TYPE_INT8, some(neg->unary.operand)->type);

    parsed_free(&p);
}

static void sema_negated_literal_adopts_type_of_other_operand()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: int8) -> uint8 { -1 + a; return 0; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(
        TYPE_INT8, some(fn_stmt(&p.root, 0, 0)->binary.lhs)->type
    );

    parsed_free(&p);
}

static void sema_assignment_has_type_of_target()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { var a: int8; a = 3; return 0; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* assign = fn_stmt(&p.root, 0, 1);
    TST_ASSERT_EQ(TYPE_INT8, assign->type);
    TST_ASSERT_EQ(TYPE_INT8, some(assign->binary.rhs)->type);

    parsed_free(&p);
}

static void sema_assignment_chains()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn f() -> uint8 { var a: uint8; var b: uint8; a = b = 3; return a; }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(TYPE_UINT8, fn_stmt(&p.root, 0, 2)->type);

    parsed_free(&p);
}

static void sema_builtin_symbol_is_synthesised()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return @read(1); }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* builtin = some(fn_stmt(&p.root, 0, 0)->ret.expr);
    TST_ASSERT_EQ(TYPE_UINT8, builtin->type);
    TST_ASSERT(builtin->symbol != NULL);
    if (builtin->symbol != NULL) {
        TST_ASSERT_EQ(SYM_BUILTIN, builtin->symbol->kind);
        TST_ASSERT_EQ(BUILTIN_READ, builtin->symbol->builtin_kind);
        TST_ASSERT_EQ((size_t)1, builtin->symbol->paramlist.count);
    }

    parsed_free(&p);
}

static void sema_builtin_name_is_not_reserved()
{
    struct Parsed p;
    // '@' opens its own namespace, so 'read' stays free for functions
    checked_init(
        &p,
        "fn read() -> uint8 { return 0; }"
        "fn f() -> uint8 { return @read(read()); }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    parsed_free(&p);
}

static void sema_nested_block_sees_outer_variable()
{
    struct Parsed p;
    // also: a return inside a nested block satisfies the return path check
    checked_init(&p, "fn f() -> uint8 { var a: uint8 = 1; { return a; } }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    parsed_free(&p);
}

static void sema_sibling_blocks_may_reuse_names()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn f() -> uint8 { { var a: uint8; } { var a: int8; } return 0; }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    parsed_free(&p);
}

static void sema_none_function_needs_no_return()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> none { }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    parsed_free(&p);
}

int main()
{
    TST_RUN(sema_empty_program);
    TST_RUN(sema_annotates_function_signature);
    TST_RUN(sema_ident_resolves_to_its_declaration);
    TST_RUN(sema_call_resolves_function_defined_later);
    TST_RUN(sema_literal_adopts_expected_type);
    TST_RUN(sema_literal_without_context_is_uint8);
    TST_RUN(sema_literal_adopts_type_of_other_operand);
    TST_RUN(sema_negated_literal_without_context_is_int8);
    TST_RUN(sema_negated_literal_adopts_type_of_other_operand);
    TST_RUN(sema_assignment_has_type_of_target);
    TST_RUN(sema_assignment_chains);
    TST_RUN(sema_builtin_symbol_is_synthesised);
    TST_RUN(sema_builtin_name_is_not_reserved);
    TST_RUN(sema_nested_block_sees_outer_variable);
    TST_RUN(sema_sibling_blocks_may_reuse_names);
    TST_RUN(sema_none_function_needs_no_return);
    TST_SUMMARY();
}
// @claude-end

// @claude-begin
// Broken inputs only: every program here parses, but must be rejected by
// sema.
//
// The property under test throughout is that one broken construct yields
// exactly one diagnostic. TYPE_INVALID is sema's error channel: whoever
// receives it stays silent, because the diagnostic was already reported.
// A test that sees a second diagnostic has found a cascade.

#include "tst.h"
#include "unit/sema/sema_util.h"

static void report_unknown_type_once()
{
    struct Parsed p;
    // 'a' is still declared, so neither use reports anything further
    checked_init(&p, "fn f() -> uint8 { var a: foo; a = 1; return a; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_INVALID_TYPE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_variable_used_as_type()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: uint8) -> uint8 { var b: a; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_INVALID_TYPE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_none_as_variable_type()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { var a: none = 5; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_INVALID_TYPE_USAGE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_none_as_parameter_type()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: none) -> uint8 { return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_INVALID_TYPE_USAGE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_undeclared_identifier()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return x; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_initialiser_reading_its_own_variable()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { var a: uint8 = a; return a; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_one_diagnostic_per_broken_statement()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { x; y; return 0; }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 0));
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 1));

    parsed_free(&p);
}

static void report_redeclaration_in_same_scope()
{
    struct Parsed p;
    checked_init(
        &p, "fn f() -> uint8 { var a: uint8; var a: uint8; return 0; }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_REDECLARATION, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_shadowing_of_outer_variable()
{
    struct Parsed p;
    checked_init(
        &p, "fn f() -> uint8 { var a: uint8; { var a: uint8; } return 0; }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_REDECLARATION, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_local_colliding_with_parameter()
{
    struct Parsed p;
    // parameters share their scope with the body's top-level locals
    checked_init(&p, "fn f(a: uint8) -> uint8 { var a: uint8; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_REDECLARATION, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_duplicate_function()
{
    struct Parsed p;
    checked_init(
        &p, "fn f() -> uint8 { return 0; } fn f() -> uint8 { return 0; }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_REDECLARATION, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_function_used_as_variable()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return f; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_A_VARIABLE, diag_at(&p, 0));

    // the name was found, only its kind is wrong: the symbol stays annotated
    const struct Node* ident = some(fn_stmt(&p.root, 0, 0)->ret.expr);
    TST_ASSERT(ident->symbol != NULL && ident->symbol->kind == SYM_FUNC);
    TST_ASSERT_EQ(TYPE_INVALID, ident->type);

    parsed_free(&p);
}

static void report_type_used_as_variable()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return uint8; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_A_VARIABLE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_call_of_variable()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: uint8) -> uint8 { return a(1); }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_A_FUNCTION, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_unknown_builtin_and_still_check_its_arguments()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return @foo(x); }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNKNOWN_BUILTIN, diag_at(&p, 0));
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 1));

    parsed_free(&p);
}

static void report_argument_count_mismatch()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn g(a: uint8) -> uint8 { return a; }"
        "fn f() -> uint8 { return g(1, 2); }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_ARG_COUNT_MISMATCH, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_argument_type_mismatch()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn g(a: uint8) -> uint8 { return a; }"
        "fn f(b: int8) -> uint8 { return g(b); }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_TYPE_MISMATCH, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_return_type_mismatch()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: int8) -> uint8 { return a; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_TYPE_MISMATCH, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_mismatched_operands_once()
{
    struct Parsed p;
    // 'a' mismatches, which leaves 'b' without an expectation
    checked_init(&p, "fn f(a: int8, b: int8) -> uint8 { return a + b; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_TYPE_MISMATCH, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_literal_out_of_range()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return 256; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_LITERAL_OUT_OF_RANGE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_literal_out_of_range_of_adopted_type()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> int8 { return 128; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_LITERAL_OUT_OF_RANGE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_negation_of_unsigned_literal()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { return -1; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_ILLEGAL_OPERATION_ON_TYPE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_negation_of_unsigned_variable()
{
    struct Parsed p;
    checked_init(&p, "fn f(a: uint8) -> uint8 { -a; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_ILLEGAL_OPERATION_ON_TYPE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_arithmetic_on_none()
{
    struct Parsed p;
    checked_init(
        &p,
        "fn g() -> none { }"
        "fn f() -> uint8 { g() + 1; return 0; }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_ILLEGAL_OPERATION_ON_TYPE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_value_returned_from_none_function()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> none { return 1; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_TYPE_MISMATCH, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_undeclared_target_as_undeclared()
{
    struct Parsed p;
    // not additionally as "not assignable"
    checked_init(&p, "fn f() -> uint8 { x = 1; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNDECLARED, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_function_as_target_once()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { f = 1; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_A_VARIABLE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_literal_as_target()
{
    struct Parsed p;
    // 'a' is int8, the literal uint8: no follow-up mismatch on the value
    checked_init(&p, "fn f() -> uint8 { var a: int8; 5 = a; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_ASSIGNABLE, diag_at(&p, 0));
    TST_ASSERT_EQ(32u, p.dl.count > 0 ? p.dl.items[0].loc.col : 0u);

    parsed_free(&p);
}

static void report_binary_as_target()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { var a: uint8; a + a = 1; return 0; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_ASSIGNABLE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_assignment_as_target()
{
    struct Parsed p;
    checked_init(
        &p, "fn f() -> uint8 { var a: uint8; (a = a) = 1; return 0; }"
    );

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_NOT_ASSIGNABLE, diag_at(&p, 0));

    parsed_free(&p);
}

static void report_missing_return()
{
    struct Parsed p;
    checked_init(&p, "fn f() -> uint8 { var a: uint8; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_MISSING_RETURN, diag_at(&p, 0));

    parsed_free(&p);
}

int main()
{
    TST_RUN(report_unknown_type_once);
    TST_RUN(report_variable_used_as_type);
    TST_RUN(report_none_as_variable_type);
    TST_RUN(report_none_as_parameter_type);
    TST_RUN(report_undeclared_identifier);
    TST_RUN(report_initialiser_reading_its_own_variable);
    TST_RUN(report_one_diagnostic_per_broken_statement);
    TST_RUN(report_redeclaration_in_same_scope);
    TST_RUN(report_shadowing_of_outer_variable);
    TST_RUN(report_local_colliding_with_parameter);
    TST_RUN(report_duplicate_function);
    TST_RUN(report_function_used_as_variable);
    TST_RUN(report_type_used_as_variable);
    TST_RUN(report_call_of_variable);
    TST_RUN(report_unknown_builtin_and_still_check_its_arguments);
    TST_RUN(report_argument_count_mismatch);
    TST_RUN(report_argument_type_mismatch);
    TST_RUN(report_return_type_mismatch);
    TST_RUN(report_mismatched_operands_once);
    TST_RUN(report_literal_out_of_range);
    TST_RUN(report_literal_out_of_range_of_adopted_type);
    TST_RUN(report_negation_of_unsigned_literal);
    TST_RUN(report_negation_of_unsigned_variable);
    TST_RUN(report_arithmetic_on_none);
    TST_RUN(report_value_returned_from_none_function);
    TST_RUN(report_undeclared_target_as_undeclared);
    TST_RUN(report_function_as_target_once);
    TST_RUN(report_literal_as_target);
    TST_RUN(report_binary_as_target);
    TST_RUN(report_assignment_as_target);
    TST_RUN(report_missing_return);
    TST_SUMMARY();
}
// @claude-end

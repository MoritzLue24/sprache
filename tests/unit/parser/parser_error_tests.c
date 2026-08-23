// Broken inputs only: error reporting and panic-mode recovery.
//
// Two properties are under test throughout. First, TERMINATION -- every
// collecting loop must consume at least one token per iteration, so a
// test that hangs is a test that failed. Second, that one broken
// construct yields exactly one diagnostic, because p->panic suppresses
// the follow-up noise and the collecting loops clear it again at the next
// item boundary.

#include "tst.h"
#include "unit/parser/parse_util.h"

static void recover_does_not_loop_on_missing_operand()
{
    struct Parsed p;
    // Regression: this input used to hang the parser. parse_factor()
    // rejects '*' without consuming it and parse_stmt()'s expect(';')
    // does not consume either, so without sync() the block loop retries
    // the same token forever.
    parsed_init(&p, "fn main() {2+*1;}");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_EXPRESSION_EXPECTED, p.dl.items[0].code);
    TST_ASSERT_EQ((size_t)1, p.root.program.nl.count);
    TST_ASSERT_EQ((size_t)0, fn_body(&p.root, 0)->block.nl.count);

    parsed_free(&p);
}

static void report_suppresses_follow_up_diagnostics()
{
    struct Parsed p;
    // Two things are wrong here -- the missing operand and the ';' that
    // expect() then trips over -- but they are one broken statement.
    parsed_init(&p, "fn main() { 1 + ; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_EXPRESSION_EXPECTED, p.dl.items[0].code);

    parsed_free(&p);
}

static void report_one_diagnostic_per_broken_statement()
{
    struct Parsed p;
    // The block loop clears p->panic before each statement, so the second
    // one is reported again instead of being swallowed.
    parsed_init(&p, "fn main() { ; ; }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);

    parsed_free(&p);
}

static void report_panic_clears_between_top_level_items()
{
    struct Parsed p;
    parsed_init(&p, "fn a() { ; } fn b() { ; }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);
    TST_ASSERT_EQ((size_t)2, p.root.program.nl.count);

    parsed_free(&p);
}

static void report_diagnostic_carries_code_and_location()
{
    struct Parsed p;
    // col 22 is the '}' that should have been a ';'
    parsed_init(&p, "fn main() { return a }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_DIFFERENT_TOKEN_EXPECTED, p.dl.items[0].code);
    TST_ASSERT_EQ(1u, p.dl.items[0].loc.line);
    TST_ASSERT_EQ(22u, p.dl.items[0].loc.col);

    parsed_free(&p);
}

static void recover_keeps_following_statement()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { 1 + ; return 2; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    // The broken expression is gone, the intact return survived.
    TST_ASSERT_EQ((size_t)1, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 0, 0)->kind);

    parsed_free(&p);
}

static void recover_keeps_node_when_only_separator_is_missing()
{
    struct Parsed p;
    // A missing ';' does not put the meaning of the statement in doubt,
    // so the node stays and only the separator is reported.
    parsed_init(&p, "fn main() { var a = 2 return a; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)2, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_VAR_DEF, fn_stmt(&p.root, 0, 0)->kind);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 0, 1)->kind);

    parsed_free(&p);
}

static void recover_keeps_last_statement_without_semicolon()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { return a }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)1, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 0, 0)->kind);

    parsed_free(&p);
}

static void recover_at_toplevel_skips_junk_before_function()
{
    struct Parsed p;
    parsed_init(&p, "1 2 3 fn main() { }");

    // SYNC_TOPLEVEL walks to the next 'fn' and stops in front of it.
    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_UNEXPECTED_TOKEN, p.dl.items[0].code);
    TST_ASSERT_EQ((size_t)1, p.root.program.nl.count);
    TST_ASSERT(strcmp(val(at(&p.root.program.nl, 0)), "main") == 0);

    parsed_free(&p);
}

static void recover_does_not_swallow_next_function()
{
    struct Parsed p;
    // TK_FUNC is a stop token on every sync level. If it were not, the
    // recovery inside a's body would eat b along with it.
    parsed_init(&p, "fn a() { ) } fn b() { return 1; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)2, p.root.program.nl.count);
    TST_ASSERT_EQ((size_t)0, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 1, 0)->kind);

    parsed_free(&p);
}

static void recover_in_nested_block_keeps_outer_statements()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { { ) } return 1; }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)2, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_BLOCK, fn_stmt(&p.root, 0, 0)->kind);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 0, 1)->kind);

    parsed_free(&p);
}

static void recover_in_parameter_list_keeps_later_params()
{
    struct Parsed p;
    // SYNC_PARAM stops in front of the ',' and leaves it to the loop.
    parsed_init(&p, "fn f(a, 1, b) { }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ((size_t)2, fn->func_def.params.count);
    TST_ASSERT(strcmp(val(at(&fn->func_def.params, 0)), "a") == 0);
    TST_ASSERT(strcmp(val(at(&fn->func_def.params, 1)), "b") == 0);

    parsed_free(&p);
}

static void report_one_diagnostic_per_broken_parameter()
{
    struct Parsed p;
    parsed_init(&p, "fn f(1, 2) { }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);
    TST_ASSERT_EQ((size_t)0, at(&p.root.program.nl, 0)->func_def.params.count);

    parsed_free(&p);
}

static void recover_in_argument_list_keeps_later_args()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { f(, 1); }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);

    const struct Node* call = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_CALL, call->kind);
    TST_ASSERT_EQ((size_t)1, call->call.args.count);
    TST_ASSERT(strcmp(val(at(&call->call.args, 0)), "1") == 0);

    parsed_free(&p);
}

static void recover_from_missing_assign_in_var_definition()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { var a 2; return 1; }");

    // SYNC_STATEMENT consumes through the ';' that ends the broken
    // statement, so the return is parsed from a clean position.
    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)1, fn_body(&p.root, 0)->block.nl.count);
    TST_ASSERT_EQ(NODE_RETURN, fn_stmt(&p.root, 0, 0)->kind);

    parsed_free(&p);
}

static void recover_from_empty_parenthesised_expression()
{
    struct Parsed p;
    parsed_init(&p, "fn main() { (); }");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ(DIAG_EXPRESSION_EXPECTED, p.dl.items[0].code);
    TST_ASSERT_EQ((size_t)0, fn_body(&p.root, 0)->block.nl.count);

    parsed_free(&p);
}

static void recover_from_unclosed_block_at_eof()
{
    struct Parsed p;
    // advance() clamps at TK_END, so sync() has to stop there by itself
    // or it spins on the last token.
    parsed_init(&p, "fn main() {");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)0, p.root.program.nl.count);

    parsed_free(&p);
}

static void recover_from_incomplete_header_at_eof()
{
    struct Parsed p;
    parsed_init(&p, "fn");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)0, p.root.program.nl.count);

    parsed_free(&p);
}

static void recover_from_junk_without_any_function()
{
    struct Parsed p;
    // One diagnostic, not three: p->panic stays set while SYNC_TOPLEVEL
    // walks the rest of the file looking for an 'fn' that never comes.
    parsed_init(&p, "1 2 3");

    TST_ASSERT_EQ((size_t)1, p.dl.count);
    TST_ASSERT_EQ((size_t)0, p.root.program.nl.count);

    parsed_free(&p);
}

static void recover_from_unclosed_paren_in_signature()
{
    struct Parsed p;
    // KNOWN ROUGH EDGE: sync(SYNC_PARAM) clears p->panic before
    // parse_func_def's expect(')') runs, so this reports twice at the
    // same column. Lower the count once that is fixed.
    parsed_init(&p, "fn main( { }");

    TST_ASSERT_EQ((size_t)2, p.dl.count);
    TST_ASSERT_EQ(p.dl.items[0].loc.col, p.dl.items[1].loc.col);

    parsed_free(&p);
}

int main()
{
    TST_RUN(recover_does_not_loop_on_missing_operand);
    TST_RUN(report_suppresses_follow_up_diagnostics);
    TST_RUN(report_one_diagnostic_per_broken_statement);
    TST_RUN(report_panic_clears_between_top_level_items);
    TST_RUN(report_diagnostic_carries_code_and_location);
    TST_RUN(recover_keeps_following_statement);
    TST_RUN(recover_keeps_node_when_only_separator_is_missing);
    TST_RUN(recover_keeps_last_statement_without_semicolon);
    TST_RUN(recover_at_toplevel_skips_junk_before_function);
    TST_RUN(recover_does_not_swallow_next_function);
    TST_RUN(recover_in_nested_block_keeps_outer_statements);
    TST_RUN(recover_in_parameter_list_keeps_later_params);
    TST_RUN(report_one_diagnostic_per_broken_parameter);
    TST_RUN(recover_in_argument_list_keeps_later_args);
    TST_RUN(recover_from_missing_assign_in_var_definition);
    TST_RUN(recover_from_empty_parenthesised_expression);
    TST_RUN(recover_from_unclosed_block_at_eof);
    TST_RUN(recover_from_incomplete_header_at_eof);
    TST_RUN(recover_from_junk_without_any_function);
    TST_RUN(recover_from_unclosed_paren_in_signature);
    TST_SUMMARY();
}

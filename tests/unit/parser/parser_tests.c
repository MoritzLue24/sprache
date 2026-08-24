// Valid inputs only: every construct the parser currently accepts has to
// end up in the AST with the right shape, and without a single diagnostic.
// Broken inputs live in "parser_error_tests.c".

#include "tst.h"
#include "unit/parser/parse_util.h"

static void parse_empty_program()
{
    struct Parsed p;
    parsed_init(&p, "");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(NODE_PROGRAM, p.root.kind);
    TST_ASSERT_EQ((size_t)0, p.root.program.nl.count);

    parsed_free(&p);
}

static void parse_function_with_empty_body()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ((size_t)1, p.root.program.nl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ(NODE_FUNC_DEF, fn->kind);
    TST_ASSERT(strcmp(val(fn), "main") == 0);
    TST_ASSERT_EQ((size_t)0, fn->func_def.params.count);
    TST_ASSERT_EQ(NODE_BLOCK, fn_body(&p.root, 0)->kind);
    TST_ASSERT_EQ((size_t)0, fn_body(&p.root, 0)->block.nl.count);

    parsed_free(&p);
}

static void parse_multiple_top_level_functions()
{
    struct Parsed p;
    parsed_init(
        &p, "fn a() -> uint8 { } fn b() -> uint8 { } fn c() -> uint8 { }"
    );

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ((size_t)3, p.root.program.nl.count);
    TST_ASSERT(strcmp(val(at(&p.root.program.nl, 0)), "a") == 0);
    TST_ASSERT(strcmp(val(at(&p.root.program.nl, 1)), "b") == 0);
    TST_ASSERT(strcmp(val(at(&p.root.program.nl, 2)), "c") == 0);

    parsed_free(&p);
}

static void parse_parameter_list()
{
    struct Parsed p;
    parsed_init(&p, "fn f(a: uint8, b: int8, c: bool) -> uint8 { }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ((size_t)3, fn->func_def.params.count);
    TST_ASSERT_EQ(NODE_PARAM, at(&fn->func_def.params, 0)->kind);
    TST_ASSERT(strcmp(val(at(&fn->func_def.params, 0)), "a") == 0);
    TST_ASSERT(strcmp(val(at(&fn->func_def.params, 1)), "b") == 0);
    TST_ASSERT(strcmp(val(at(&fn->func_def.params, 2)), "c") == 0);

    parsed_free(&p);
}

static void parse_function_return_type()
{
    struct Parsed p;
    parsed_init(&p, "fn f() -> int8 { }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* type = some(at(&p.root.program.nl, 0)->func_def.type);
    TST_ASSERT_EQ(NODE_TYPE, type->kind);
    TST_ASSERT(strcmp(val(type), "int8") == 0);

    parsed_free(&p);
}

static void parse_parameter_types()
{
    struct Parsed p;
    parsed_init(&p, "fn f(a: uint8, b: int8) -> bool { }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ((size_t)2, fn->func_def.params.count);

    const struct Node* a = some(at(&fn->func_def.params, 0)->param.type);
    TST_ASSERT_EQ(NODE_TYPE, a->kind);
    TST_ASSERT(strcmp(val(a), "uint8") == 0);

    const struct Node* b = some(at(&fn->func_def.params, 1)->param.type);
    TST_ASSERT(strcmp(val(b), "int8") == 0);

    parsed_free(&p);
}

static void parse_type_is_an_ordinary_ident()
{
    struct Parsed p;
    // Types are no longer keywords, so the parser takes whatever
    // identifier it finds and never decides whether it names a type --
    // that is sema's job.
    parsed_init(&p, "fn f(a: Led) -> Matrix { var b: whatever; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    const struct Node* param = at(&fn->func_def.params, 0);
    const struct Node* v = fn_stmt(&p.root, 0, 0);

    TST_ASSERT(strcmp(val(some(fn->func_def.type)), "Matrix") == 0);
    TST_ASSERT(strcmp(val(some(param->param.type)), "Led") == 0);
    TST_ASSERT(strcmp(val(some(v->var_decl.type)), "whatever") == 0);

    parsed_free(&p);
}

static void parse_type_names_are_not_reserved()
{
    struct Parsed p;
    // 'uint8' lexes as TK_IDENT like any other name, so nothing stops it
    // from naming a variable. Rejecting that is sema's call, not the
    // parser's.
    parsed_init(&p, "fn main() -> uint8 { var uint8: uint8; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* v = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_VAR_DECL, v->kind);
    TST_ASSERT(strcmp(val(v), "uint8") == 0);
    TST_ASSERT(strcmp(val(some(v->var_decl.type)), "uint8") == 0);

    parsed_free(&p);
}

static void parse_type_tracks_source_location()
{
    struct Parsed p;
    // The type node points at the name itself, not at the '->' or ':'
    // that introduced it: col 14 is 'uint8', col 29 is 'int8'.
    parsed_init(&p, "fn main() -> uint8 { var a: int8; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ(14u, some(fn->func_def.type)->loc.col);

    const struct Node* v = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(29u, some(v->var_decl.type)->loc.col);

    parsed_free(&p);
}

static void parse_nested_blocks()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { { { 1; } } }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* outer = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_BLOCK, outer->kind);

    const struct Node* inner = at(&outer->block.nl, 0);
    TST_ASSERT_EQ(NODE_BLOCK, inner->kind);
    TST_ASSERT_EQ(NODE_LITERAL, at(&inner->block.nl, 0)->kind);

    parsed_free(&p);
}

static void parse_var_declaration_without_init()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { var a: int8; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* v = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_VAR_DECL, v->kind);
    TST_ASSERT(strcmp(val(v), "a") == 0);
    TST_ASSERT_EQ(NODE_TYPE, some(v->var_decl.type)->kind);
    TST_ASSERT(strcmp(val(some(v->var_decl.type)), "int8") == 0);

    parsed_free(&p);
}

static void parse_var_definition_with_init()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { var a: uint8 = 1 + 2; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* v = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_VAR_DEF, v->kind);
    TST_ASSERT(strcmp(val(v), "a") == 0);
    TST_ASSERT(strcmp(val(some(v->var_def.type)), "uint8") == 0);
    TST_ASSERT_EQ(NODE_BINARY, some(v->var_def.init)->kind);
    TST_ASSERT_EQ(OP_PLUS, some(v->var_def.init)->op);

    parsed_free(&p);
}

static void parse_return_statement()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { return 1; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* r = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_RETURN, r->kind);
    TST_ASSERT_EQ(NODE_LITERAL, some(r->ret.expr)->kind);
    TST_ASSERT(strcmp(val(some(r->ret.expr)), "1") == 0);

    parsed_free(&p);
}

static void parse_expression_statement()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { 42; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* e = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_LITERAL, e->kind);
    TST_ASSERT(strcmp(val(e), "42") == 0);

    parsed_free(&p);
}

static void parse_ident_is_not_a_call()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { a; a(); }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(NODE_IDENT, fn_stmt(&p.root, 0, 0)->kind);
    TST_ASSERT_EQ(NODE_CALL, fn_stmt(&p.root, 0, 1)->kind);
    TST_ASSERT_EQ((size_t)0, fn_stmt(&p.root, 0, 1)->call.args.count);

    parsed_free(&p);
}

static void parse_call_arguments()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { g(1); h(1, a, 2 + 3); }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* g = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_CALL, g->kind);
    TST_ASSERT(strcmp(val(g), "g") == 0);
    TST_ASSERT_EQ((size_t)1, g->call.args.count);

    const struct Node* h = fn_stmt(&p.root, 0, 1);
    TST_ASSERT_EQ((size_t)3, h->call.args.count);
    TST_ASSERT_EQ(NODE_LITERAL, at(&h->call.args, 0)->kind);
    TST_ASSERT_EQ(NODE_IDENT, at(&h->call.args, 1)->kind);
    TST_ASSERT_EQ(NODE_BINARY, at(&h->call.args, 2)->kind);

    parsed_free(&p);
}

static void parse_builtin_call()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { @out(1, 2); }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* b = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_BUILTIN, b->kind);
    TST_ASSERT(strcmp(val(b), "out") == 0);
    TST_ASSERT_EQ((size_t)2, b->builtin.args.count);

    parsed_free(&p);
}

static void parse_unary_operators()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { -1; ~a; - ~ b; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* neg = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_UNARY, neg->kind);
    TST_ASSERT_EQ(OP_NEG, neg->op);
    TST_ASSERT_EQ(NODE_LITERAL, some(neg->unary.operand)->kind);

    const struct Node* bnot = fn_stmt(&p.root, 0, 1);
    TST_ASSERT_EQ(OP_BW_NOT, bnot->op);
    TST_ASSERT_EQ(NODE_IDENT, some(bnot->unary.operand)->kind);

    // A prefix chain nests to the right without any table saying so:
    // parse_unary() recurses into itself.
    const struct Node* chain = fn_stmt(&p.root, 0, 2);
    TST_ASSERT_EQ(OP_NEG, chain->op);
    TST_ASSERT_EQ(NODE_UNARY, some(chain->unary.operand)->kind);
    TST_ASSERT_EQ(OP_BW_NOT, some(chain->unary.operand)->op);

    parsed_free(&p);
}

static void parse_binary_precedence()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { 1 + 2 * 3; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    // '*' binds tighter, so it has to sit below the '+'.
    const struct Node* add = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(NODE_BINARY, add->kind);
    TST_ASSERT_EQ(OP_PLUS, add->op);
    TST_ASSERT_EQ(NODE_LITERAL, some(add->binary.lhs)->kind);
    TST_ASSERT_EQ(OP_MUL, some(add->binary.rhs)->op);

    parsed_free(&p);
}

static void parse_left_associativity()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { 1 - 2 - 3; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    // ((1 - 2) - 3): equal precedence folds to the left.
    const struct Node* top = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(OP_MINUS, top->op);
    TST_ASSERT(strcmp(val(some(top->binary.rhs)), "3") == 0);

    const struct Node* left = some(top->binary.lhs);
    TST_ASSERT_EQ(NODE_BINARY, left->kind);
    TST_ASSERT_EQ(OP_MINUS, left->op);
    TST_ASSERT(strcmp(val(some(left->binary.lhs)), "1") == 0);
    TST_ASSERT(strcmp(val(some(left->binary.rhs)), "2") == 0);

    parsed_free(&p);
}

static void parse_right_associativity()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { a = b = c; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    // (a = (b = c)): the same precedence, but ASSOC_RIGHT keeps the
    // second '=' inside the recursion instead of handing it back.
    const struct Node* top = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(OP_ASSIGN, top->op);
    TST_ASSERT(strcmp(val(some(top->binary.lhs)), "a") == 0);

    const struct Node* right = some(top->binary.rhs);
    TST_ASSERT_EQ(NODE_BINARY, right->kind);
    TST_ASSERT_EQ(OP_ASSIGN, right->op);
    TST_ASSERT(strcmp(val(some(right->binary.lhs)), "b") == 0);
    TST_ASSERT(strcmp(val(some(right->binary.rhs)), "c") == 0);

    parsed_free(&p);
}

static void parse_full_precedence_ladder()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { a | b ^ c & d + e * f; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    // Every level of binop.def in one expression, weakest on top:
    // (a | (b ^ (c & (d + (e * f)))))
    const struct Node* n = fn_stmt(&p.root, 0, 0);
    enum OpKind expected[] = {
        OP_BW_OR, OP_BW_XOR, OP_BW_AND, OP_PLUS, OP_MUL
    };

    for (size_t k = 0; k < sizeof(expected) / sizeof(*expected); k++) {
        TST_ASSERT_EQ(NODE_BINARY, n->kind);
        TST_ASSERT_EQ(expected[k], n->op);
        TST_ASSERT_EQ(NODE_IDENT, some(n->binary.lhs)->kind);
        n = some(n->binary.rhs);
    }
    TST_ASSERT_EQ(NODE_IDENT, n->kind);
    TST_ASSERT(strcmp(val(n), "f") == 0);

    parsed_free(&p);
}

static void parse_parentheses_override_precedence()
{
    struct Parsed p;
    parsed_init(&p, "fn main() -> uint8 { (1 + 2) * 3; }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* mul = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(OP_MUL, mul->op);
    TST_ASSERT_EQ(OP_PLUS, some(mul->binary.lhs)->op);
    TST_ASSERT_EQ(NODE_LITERAL, some(mul->binary.rhs)->kind);

    parsed_free(&p);
}

static void parse_parenthesis_moves_loc_to_lparen()
{
    struct Parsed p;
    // col 22 is '(', col 23 is '1'
    parsed_init(&p, "fn main() -> uint8 { (1); }");

    TST_ASSERT_EQ((size_t)0, p.dl.count);
    TST_ASSERT_EQ(NODE_LITERAL, fn_stmt(&p.root, 0, 0)->kind);
    TST_ASSERT_EQ(22u, fn_stmt(&p.root, 0, 0)->loc.col);

    parsed_free(&p);
}

static void parse_tracks_source_locations()
{
    struct Parsed p;
    // fn main() -> uint8 {
    //     return 1;
    // }
    parsed_init(&p, "fn main() -> uint8 {\n    return 1;\n}");

    TST_ASSERT_EQ((size_t)0, p.dl.count);

    const struct Node* fn = at(&p.root.program.nl, 0);
    TST_ASSERT_EQ(1u, fn->loc.line);
    TST_ASSERT_EQ(1u, fn->loc.col);
    TST_ASSERT_EQ(20u, fn_body(&p.root, 0)->loc.col);

    const struct Node* r = fn_stmt(&p.root, 0, 0);
    TST_ASSERT_EQ(2u, r->loc.line);
    TST_ASSERT_EQ(5u, r->loc.col);
    TST_ASSERT_EQ(12u, some(r->ret.expr)->loc.col);

    parsed_free(&p);
}

int main()
{
    TST_RUN(parse_empty_program);
    TST_RUN(parse_function_with_empty_body);
    TST_RUN(parse_multiple_top_level_functions);
    TST_RUN(parse_parameter_list);
    TST_RUN(parse_function_return_type);
    TST_RUN(parse_parameter_types);
    TST_RUN(parse_type_is_an_ordinary_ident);
    TST_RUN(parse_type_names_are_not_reserved);
    TST_RUN(parse_type_tracks_source_location);
    TST_RUN(parse_nested_blocks);
    TST_RUN(parse_var_declaration_without_init);
    TST_RUN(parse_var_definition_with_init);
    TST_RUN(parse_return_statement);
    TST_RUN(parse_expression_statement);
    TST_RUN(parse_ident_is_not_a_call);
    TST_RUN(parse_call_arguments);
    TST_RUN(parse_builtin_call);
    TST_RUN(parse_unary_operators);
    TST_RUN(parse_binary_precedence);
    TST_RUN(parse_left_associativity);
    TST_RUN(parse_right_associativity);
    TST_RUN(parse_full_precedence_ladder);
    TST_RUN(parse_parentheses_override_precedence);
    TST_RUN(parse_parenthesis_moves_loc_to_lparen);
    TST_RUN(parse_tracks_source_locations);
    TST_SUMMARY();
}

#include "frontend/parser/parser.h"
#include "frontend/parser/parser_internal.h"

#include <assert.h>
#include <string.h>

#include "utils/xalloc.h"
#include "frontend/core/loc.h"
#include "frontend/parser/ast.h"


// forward decl
static struct Node* parse_top_level_item();
static struct Node* parse_node_func_def();
static struct Node* parse_node_block();
static struct Node* parse_statement();
static struct Node* parse_node_var_decl_or_def();
static struct Node* parse_node_return();
static struct Node* parse_expr();
static struct Node* parse_sum();
static struct Node* parse_term();
static struct Node* parse_factor();
static struct Node* parse_node_builtin();
static struct Node* parse_node_var();
static struct Node* parse_node_call();


struct Node* parse(const struct Token* tokens_head, struct ErrorList* errorlist)
{
    struct Parser p;
    init_parser(&p, tokens_head, errorlist);
    set_ctx(&p);

    struct Node* n_program = alloc_node(NODE_PROGRAM, peek()->begin);
    init_nodelist(&n_program->program.items);

    while (peek()->type != TT_END) {
        struct Node* cur_node = parse_top_level_item();
        nodelist_push(&n_program->program.items, cur_node);
    }

    unset_ctx();
    return n_program;
}

static struct Node* parse_top_level_item()
{
    switch (peek()->type) {
        case TT_FUNC:
            return parse_node_func_def();

        default:
            const struct Token* t = advance();
            add_error(
                errors_ptr(), ERROR_SYNTAX, t->begin,
                "Unexpected token: '%s'", tt_str(t->type)
            );
            return alloc_invalid_node();
    }
}

static struct Node* parse_node_func_def()
{
    assert(check(TT_FUNC));
    struct Loc begin = peek()->begin;
    advance();

    const struct Token* ident_t = advance();
    assert(ident_t->value);


    // parameter list
    struct NodeList params;
    init_nodelist(&params);

    expect(TT_LPAREN);
    if (!check(TT_RPAREN)) {
        while (true) {
            const struct Token* param_t = expect(TT_IDENT);

            struct Node* param_n;
            if (param_t == NULL) {
                advance();
                set_need_sync();
                param_n = alloc_invalid_node();
            }
            else {
                param_n = alloc_node(NODE_PARAM, param_t->begin);
                param_n->param.ident = xstrdup(param_t->value);
            }
            nodelist_push(&params, param_n);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }
    expect(TT_RPAREN);

    // body
    struct Node* body_n = parse_node_block();

    struct Node* func_def_n = alloc_node(NODE_FUNC_DEF, begin);
    func_def_n->func_def.ident = xstrdup(ident_t->value);
    func_def_n->func_def.params = params;
    func_def_n->func_def.body = body_n;
    return func_def_n;
}

static struct Node* parse_node_block()
{
    assert(check(TT_LBRACE));
    struct Loc begin = peek()->begin;
    advance();

    struct NodeList nl;
    init_nodelist(&nl);

    while (!check(TT_END) && !check(TT_RBRACE)) {
        struct Node* cur_node = parse_statement();
        assert(!need_sync());   // FIXME: for testing
        nodelist_push(&nl, cur_node);
    }
    expect(TT_RBRACE);

    struct Node* block_n = alloc_node(NODE_BLOCK, begin);
    block_n->block.statements = nl;
    return block_n;
}

static struct Node* parse_statement()
{
    struct Node* node;
    switch (peek()->type) {
        case TT_LBRACE:
            node = parse_node_block();
            break;

        case TT_VAR:
            node = parse_node_var_decl_or_def();
            break;

        case TT_RETURN:
            node = parse_node_return();
            break;

        default:
            node = parse_expr();
            expect(TT_SEMICOLON);
    }
    if (need_sync()) {
        sync();
    }
    return node;
}

static struct Node* parse_node_var_decl_or_def()
{
    assert(check(TT_VAR));
    struct Loc begin = peek()->begin;
    advance();

    struct Node* target = parse_node_var();
    struct Node* node;

    if (check(TT_SEMICOLON)) {
        node = alloc_node(NODE_VAR_DECL, begin);
        node->var_decl.target = target;
    }
    else {
        expect(TT_EQ);

        struct Node* expr = parse_expr();
        node = alloc_node(NODE_VAR_DEF, begin);
        node->var_def.target = target;
        node->var_def.expr = expr;
    }
    expect(TT_SEMICOLON);
    return node;
}

static struct Node* parse_node_return()
{
    assert(check(TT_RETURN));
    struct Node* node = alloc_node(NODE_RETURN, peek()->begin);
    advance();

    node->ret.expr = parse_expr();

    expect(TT_SEMICOLON);
    return node;
}

static struct Node* parse_expr()
{
    struct Node* node = parse_sum();

    if (need_sync())
        sync();

    if (check(TT_EQ)) {
        advance();
        struct Node* rhs = parse_expr();
        if (need_sync())
            sync();

        struct Node* n = alloc_node(NODE_ASSIGN_EXPR, node->begin);
        n->assign_expr.target = node;
        n->assign_expr.expr = rhs;
        node = n;
    }
    return node;
}

static struct Node* parse_sum()
{
    struct Node* node = parse_term();

    while (check(TT_PLUS) || check(TT_MINUS)) {
        const struct Token* op_t = advance();
        struct Node* rhs = parse_term();

        struct Node* n = alloc_node(NODE_BINARY_OP, node->begin);
        n->bin_op.op = tt_to_op(op_t->type);
        n->bin_op.lhs = node;
        n->bin_op.rhs = rhs;
        node = n;
    }
    return node;
}

static struct Node* parse_term()
{
    struct Node* node = parse_factor();

    while (check(TT_STAR)) {
        const struct Token* op_t = advance();
        struct Node* rhs = parse_factor();

        struct Node* n = alloc_node(NODE_BINARY_OP, node->begin);
        n->bin_op.op = tt_to_op(op_t->type);
        n->bin_op.lhs = node;
        n->bin_op.rhs = rhs;
        node = n;
    }
    return node;
}

static struct Node* parse_factor()
{
    if (check(TT_LITERAL)) {
        const struct Token* t = advance();
        assert(t->value);

        struct Node* node = alloc_node(NODE_LITERAL, t->begin);
        node->literal.value = xstrdup(t->value);
        return node;
    }
    else if (check(TT_IDENT)) {
        if (check_next(TT_LPAREN))
            return parse_node_call();
        return parse_node_var();
    }
    else if (check(TT_AT)) {
        return parse_node_builtin();
    }
    else if (check(TT_LPAREN)) {
        const struct Token* lparen = advance();
        struct Node* expr = parse_expr();
        expect(TT_RPAREN);

        expr->begin = lparen->begin;
        return expr;
    }
    add_error(errors_ptr(), ERROR_SYNTAX, peek()->begin, "Expected an expression: '%s'", tt_str(peek()->type));
    set_need_sync();
    return alloc_invalid_node();
}

static struct Node* parse_node_builtin()
{
    assert(check(TT_AT));
    struct Loc begin = peek()->begin;
    advance();

    const struct Token* t = advance();
    assert(t->value);

    struct NodeList args;
    init_nodelist(&args);

    struct Node* node = alloc_node(NODE_BUILTIN, begin);
    node->builtin.ident = xstrdup(t->value);
    node->builtin.def = NULL;   // set by sema

    expect(TT_LPAREN);
    if (!check(TT_RPAREN)) {
        while (true) {
            struct Node* arg = parse_expr();
            nodelist_push(&args, arg);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }
    expect(TT_RPAREN);
    node->builtin.args = args;
    return node;
}

static struct Node* parse_node_var()
{
    assert(check(TT_IDENT));
    const struct Token* t = advance();
    assert(t->value);

    struct Node* node = alloc_node(NODE_VAR, t->begin);
    node->var.ident = xstrdup(t->value);
    return node;
}

static struct Node* parse_node_call()
{
    assert(check(TT_IDENT));
    const struct Token* t = advance();
    assert(t->value);

    struct NodeList args;
    init_nodelist(&args);

    struct Node* node = alloc_node(NODE_CALL, t->begin);
    node->call.ident = xstrdup(t->value);

    expect(TT_LPAREN);
    if (!check(TT_RPAREN)) {
        while (true) {
            struct Node* arg = parse_expr();
            nodelist_push(&args, arg);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }
    expect(TT_RPAREN);
    node->call.args = args;
    return node;
}

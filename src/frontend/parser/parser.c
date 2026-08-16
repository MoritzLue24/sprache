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
static struct Node* parse_binop(
    enum TokenType t_types[],
    size_t t_count, 
    struct Node* (*parse_inner_node)(void));
static struct Node* parse_bw_or();
static struct Node* parse_bw_xor();
static struct Node* parse_bw_and();
static struct Node* parse_sum();
static struct Node* parse_term();
static struct Node* parse_unary();
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
    // init_nodelist(&n_program->program.items);
    struct Node* items_head = NULL;

    while (peek()->type != TT_END) {
        struct Node* cur_node = parse_top_level_item();
        items_head = push_node(items_head, cur_node);
        // nodelist_push(&n_program->program.items, cur_node);
    }
    n_program->program.items_head = items_head;

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

    const struct Token* ident_t = expect(TT_IDENT);
    if (ident_t == NULL) {
        set_need_sync();
        return alloc_invalid_node();
    }
    assert(ident_t->value);

    // parameter list
    // struct NodeList params;
    // init_nodelist(&params);
    struct Node* params_head = NULL;

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
                param_n->param.symbol = NULL;   // set by sema
            }
            // nodelist_push(&params, param_n);
            params_head = push_node(params_head, param_n);

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
    func_def_n->func_def.params_head = params_head;
    func_def_n->func_def.body = body_n;
    func_def_n->func_def.symbol = NULL;
    return func_def_n;
}

static struct Node* parse_node_block()
{
    struct Loc begin = peek()->begin;
    if (expect(TT_LBRACE) == NULL) {
        set_need_sync();
        return alloc_invalid_node();
    }
    struct Node* head = NULL;

    while (!check(TT_END) && !check(TT_RBRACE)) {
        const struct Token* before = peek();
        struct Node* cur_node = parse_statement();
        head = push_node(head, cur_node);
        if (peek() == before) {
            advance();
        }
    }
    expect(TT_RBRACE);

    struct Node* block_n = alloc_node(NODE_BLOCK, begin);
    block_n->block.statements_head = head;
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
    struct Node* node = parse_bw_or();

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

static struct Node* parse_binop(
    enum TokenType t_types[],
    size_t t_count, 
    struct Node* (*parse_inner_node)(void))
{
    struct Node* lhs = parse_inner_node();

    while (true) {
        bool no_op = true;
        for (size_t i = 0; i < t_count; i++) {
            if (check(t_types[i])) {
                no_op = false;
                break;
            }
        }

        if (no_op) {
            break;
        }

        const struct Token* op_t = advance();
        struct Node* rhs = parse_inner_node();
        struct Node* binop_n = alloc_node(NODE_BINARY_OP, lhs->begin);

        binop_n->bin_op.op = tt_to_op(op_t->type);
        binop_n->bin_op.lhs = lhs;
        binop_n->bin_op.rhs = rhs;
        lhs = binop_n;
    }

    return lhs;
}

static struct Node* parse_bw_or()
{
    return parse_binop((enum TokenType[]){TT_BW_OR}, 1, parse_bw_xor);
}

static struct Node* parse_bw_xor()
{
    return parse_binop((enum TokenType[]){TT_BW_XOR}, 1, parse_bw_and);
}

static struct Node* parse_bw_and()
{
    return parse_binop((enum TokenType[]){TT_BW_AND}, 1, parse_sum);
}

static struct Node* parse_sum()
{
    return parse_binop((enum TokenType[]){TT_PLUS, TT_MINUS}, 2, parse_term);
}

static struct Node* parse_term()
{
    return parse_binop((enum TokenType[]){TT_STAR}, 1, parse_unary);
}

static struct Node* parse_unary()
{
    if (check(TT_MINUS) | check(TT_BW_NOT)) {
        const struct Token* op_t = advance();
        struct Node* rhs = parse_unary();

        struct Node* n = alloc_node(NODE_UNARY_OP, op_t->begin);
        n->unary_op.op = tt_to_op(op_t->type);
        n->unary_op.factor = rhs;
        return n;
    }
    else {
        return parse_factor();
    }
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

    const struct Token* t = expect(TT_IDENT);
    if (t == NULL) {
        set_need_sync();
        return alloc_invalid_node();
    }
    assert(t->value);

    // struct NodeList args;
    // init_nodelist(&args);
    struct Node* args_head = NULL;

    struct Node* node = alloc_node(NODE_BUILTIN, begin);
    node->builtin.ident = xstrdup(t->value);
    node->builtin.def = NULL;   // set by sema

    expect(TT_LPAREN);
    if (!check(TT_RPAREN)) {
        while (true) {
            struct Node* arg = parse_expr();
            // nodelist_push(&args, arg);
            args_head = push_node(args_head, arg);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }
    expect(TT_RPAREN);
    node->builtin.args_head = args_head;
    return node;
}

static struct Node* parse_node_var()
{
    assert(check(TT_IDENT));
    const struct Token* t = advance();
    assert(t->value);

    struct Node* node = alloc_node(NODE_VAR, t->begin);
    node->var.ident = xstrdup(t->value);
    node->var.symbol = NULL;    // set by sema
    return node;
}

static struct Node* parse_node_call()
{
    assert(check(TT_IDENT));
    const struct Token* t = advance();
    assert(t->value);

    // struct NodeList args;
    // init_nodelist(&args);
    struct Node* args_head = NULL;

    struct Node* node = alloc_node(NODE_CALL, t->begin);
    node->call.ident = xstrdup(t->value);
    node->call.symbol = NULL;   // set by sema

    expect(TT_LPAREN);
    if (!check(TT_RPAREN)) {
        while (true) {
            struct Node* arg = parse_expr();
            // nodelist_push(&args, arg);
            args_head = push_node(args_head, arg);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }
    expect(TT_RPAREN);
    node->call.args_head = args_head;
    return node;
}

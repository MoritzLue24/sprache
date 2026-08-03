#include "frontend/parser/parser.h"

#include <assert.h>
#include <string.h>

#include "utils/xalloc.h"
#include "frontend/core/loc.h"
#include "frontend/parser/ast.h"


static const struct Token* tok_head = NULL;
static struct ErrorList* errors = NULL;


// helper
static const struct Token* advance()
{
    const struct Token* tmp = tok_head;
    if (tok_head != NULL && tok_head->type != TT_END)
        tok_head = tok_head->next;
    return tmp;
}

// helper
static bool check(enum TokenType tt)
{
    return tok_head != NULL && tok_head->type == tt;
}

// helper
static const struct Token* expect(enum TokenType tt)
{
    if (!check(tt)) {
        add_error(errors, ERROR_SYNTAX, tok_head->begin, "Expected: '%s', Actual: '%s'", tt_str(tt), tt_str(tok_head->type));
        return NULL;
    }
    return advance();
}

/*
static bool is_sync_token()
{
    sync_mode == SYNC_STATEMENT;
}

// helper
static void recover()
{

}*/

// helper
static struct Node* init_node(enum NodeType type, struct Loc begin)
{
    struct Node* n = xmalloc(sizeof(struct Node));
    n->type = type;
    n->begin = begin;
    return n;
}

// helper
static enum OpType tt_to_op(enum TokenType tt)
{
    switch (tt) {
        case TT_PLUS:
            return OP_PLUS;

        case TT_MINUS:
            return OP_MINUS;

        case TT_STAR:
            return OP_MUL;

        default:
            assert(0);
    }
}

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
static struct Node* parse_node_var_or_call();


struct Node* parse(const struct Token* tokens_head, struct ErrorList* errorlist)
{
    tok_head = tokens_head;
    errors = errorlist;

    struct Node* n_program = init_node(NODE_PROGRAM, tok_head->begin);
    init_nodelist(&n_program->program.items, 10);

    while (tok_head->type != TT_END) {
        struct Node* cur_node = parse_top_level_item();
        if (cur_node == NULL) {
            continue;
        }
        nodelist_push(&n_program->program.items, cur_node);
    }
    return n_program;
}

static struct Node* parse_top_level_item()
{
    switch (tok_head->type) {
        case TT_FUNC:
            return parse_node_func_def();

        default:
            const struct Token* t = advance();
            add_error(errors, ERROR_SYNTAX, t->begin, "Unexpected token: '%s'", tt_str(t->type));
            return NULL;
    }
}

static struct Node* parse_node_func_def()
{
    assert(check(TT_FUNC));

    struct Loc begin = tok_head->begin;
    advance();  // func

    const struct Token* ident_t = advance();
    if (ident_t->value == NULL)
        return NULL;

    if (expect(TT_LPAREN) == NULL)
        return NULL;

    // parameter list
    struct NodeList params;
    init_nodelist(&params, 10);

    if (!check(TT_RPAREN)) {
        while (true) {
            const struct Token* param_t = expect(TT_IDENT);
            if (param_t == NULL)
                return NULL;

            struct Node* param_n = init_node(NODE_PARAM, param_t->begin);
            param_n->param.ident = xstrdup(param_t->value);
            nodelist_push(&params, param_n);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }

    if (!expect(TT_RPAREN))
        return NULL;

    // body
    struct Node* body_n = parse_node_block();
    if (body_n == NULL)
        return NULL;

    struct Node* func_def_n = init_node(NODE_FUNC_DEF, begin);
    func_def_n->func_def.ident = xstrdup(ident_t->value);
    func_def_n->func_def.params = params;
    func_def_n->func_def.body = body_n;
    return func_def_n;
}

static struct Node* parse_node_block()
{
    struct Loc begin = tok_head->begin;
    if (!expect(TT_LBRACE))
        return NULL;

    struct NodeList nl;
    init_nodelist(&nl, 10);

    while (!check(TT_END) && !check(TT_RBRACE)) {
        struct Node* cur_node = parse_statement();
        if (cur_node != NULL) {
            nodelist_push(&nl, cur_node);
        }
    }

    if (expect(TT_RBRACE) == NULL) {
        free_nodelist(&nl);
        return NULL;
    }

    struct Node* block_n = init_node(NODE_BLOCK, begin);
    block_n->block.statements = nl;
    return block_n;
}

static struct Node* parse_statement()
{
    switch (tok_head->type) {
        case TT_RBRACE:
            return parse_node_block();

        case TT_VAR:
            return parse_node_var_decl_or_def();

        case TT_RETURN:
            return parse_node_return();

        default:
            struct Node* expr = parse_expr();
            if (!expect(TT_SEMICOLON)) {
                return NULL;
            }
            return expr;
    }
}

static struct Node* parse_node_var_decl_or_def()
{
    if (tok_head == NULL)
        return NULL;

    struct Loc begin = tok_head->begin;
    if (!expect(TT_VAR))
        return NULL;

    // var
    struct Node* target = parse_node_var_or_call();
    if (target == NULL)
        return NULL;

    struct Node* node;
    if (check(TT_SEMICOLON)) {
        node = init_node(NODE_VAR_DECL, begin);
        node->var_decl.target = target;
    }
    else {
        // EQ
        if (expect(TT_EQ) == NULL)
            return NULL;

        struct Node* expr = parse_expr();
        if (expr == NULL)
            return NULL;

        node = init_node(NODE_VAR_DEF, begin);
        node->var_def.target = target;
        node->var_def.expr = expr;
    }

    if (expect(TT_SEMICOLON) == NULL)
        return NULL;
    return node;
}

static struct Node* parse_node_return()
{
    assert(tok_head->type == TT_RETURN);
    struct Node* node = init_node(NODE_RETURN, tok_head->begin);

    // return
    advance();

    // expr
    node->ret.expr = parse_expr();
    if (node->ret.expr == NULL)
        return NULL;

    if (expect(TT_SEMICOLON) == NULL)
        return NULL;
    return node;
}

static struct Node* parse_expr()
{
    struct Node* node = parse_sum();
    if (node == NULL)
        return NULL;

    if (check(TT_EQ)) {
        advance();

        struct Node* rhs = parse_expr();

        if (rhs == NULL)
            return NULL;

        struct Node* n = init_node(NODE_ASSIGN_EXPR, node->begin);
        n->assign_expr.target = node;
        n->assign_expr.expr = rhs;
        node = n;
    }
    return node;
}

static struct Node* parse_sum()
{
    struct Node* node = parse_term();
    if (node == NULL)
        return NULL;

    while (check(TT_PLUS) || check(TT_MINUS)) {
        const struct Token* op_t = advance();
        struct Node* rhs = parse_term();

        if (rhs == NULL)
            return NULL;

        struct Node* n = init_node(NODE_BINARY_OP, node->begin);
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
    if (node == NULL)
        return NULL;

    while (check(TT_STAR)) {
        const struct Token* op_t = advance();
        struct Node* rhs = parse_factor();

        if (rhs == NULL)
            return NULL;

        struct Node* n = init_node(NODE_BINARY_OP, node->begin);
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
        if (t->value == NULL)
            return NULL;

        struct Node* node = init_node(NODE_LITERAL, t->begin);
        node->literal.value = xstrdup(t->value);
        return node;
    }
    else if (check(TT_IDENT)) {
        return parse_node_var_or_call();
    }
    else if (check(TT_AT)) {
        return parse_node_builtin();
    }
    else if (check(TT_LPAREN)) {
        const struct Token* lparen = advance();
        struct Node* expr = parse_expr();
        if (expr == NULL)
            return NULL;

        expr->begin = lparen->begin;
        if (expect(TT_RPAREN) == NULL)
            return NULL;
        return expr;
    }
    add_error(errors, ERROR_SYNTAX, tok_head->begin, "Unexpected token: '%s'", tt_str(tok_head->type));
    return NULL;
}

static struct Node* parse_node_builtin()
{
    assert(tok_head->type == TT_AT);

    struct Loc begin = tok_head->begin;
    advance();  // @

    const struct Token* t = advance();
    if (t->value == NULL)
        return NULL;

    if (expect(TT_LPAREN) == NULL)
        return NULL;

    struct NodeList args;
    init_nodelist(&args, 10);

    if (!check(TT_RPAREN)) {
        while (true) {
            struct Node* arg = parse_expr();
            if (arg == NULL)
                return NULL;

            nodelist_push(&args, arg);

            if (!check(TT_COMMA))
                break;
            advance();  // ,
        }
    }

    if (expect(TT_RPAREN) == NULL)
        return NULL;

    struct Node* node = init_node(NODE_BUILTIN, begin);
    node->builtin.ident = xstrdup(t->value);
    node->builtin.args = args;
    node->builtin.def = NULL;   // set by sema

    return node;
}

static struct Node* parse_node_var_or_call()
{
    const struct Token* t = expect(TT_IDENT);
    if (t == NULL || t->value == NULL)
        return NULL;

    struct Node* node;

    if (check(TT_LPAREN)) {
        advance();
        struct NodeList args;
        init_nodelist(&args, 10);

        if (!check(TT_RPAREN)) {
            while (true) {
                struct Node* arg = parse_expr();
                if (arg == NULL)
                    return NULL;

                nodelist_push(&args, arg);

                if (!check(TT_COMMA))
                    break;
                advance();  // ,
            }
        }

        if (expect(TT_RPAREN) == NULL)
            return NULL;

        node = init_node(NODE_CALL, t->begin);
        node->call.ident = xstrdup(t->value);
        node->call.args = args;
    }
    else {
        node = init_node(NODE_VAR, t->begin);
        node->var.ident = xstrdup(t->value);
    }
    return node;
}

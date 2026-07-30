#include "parser/parser.h"

#include <assert.h>
#include <string.h>

#include "core/loc.h"
#include "utils/xalloc.h"
#include "parser/ast.h"


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

static void skip_statement()
{
    while (!check(TT_SEMICOLON) && !check(TT_END)) {
        advance();
    }
    if (check(TT_SEMICOLON)) {
        advance();
    }
}

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
static struct Node* parse_expr();
static struct Node* parse_node_builtin();

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
        const struct Token* t = advance();
        if (t->value == NULL)
            return NULL;

        struct Node* node = init_node(NODE_VAR, t->begin);
        node->var.ident = xstrdup(t->value);
        return node;
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

static struct Node* parse_expr()
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

static struct Node* parse_node_var_decl_def()
{
    assert(tok_head->type == TT_VAR);
    struct Loc begin = tok_head->begin;
    advance();  // "var"

    // ident
    const struct Token* ident_t = expect(TT_IDENT);
    if (ident_t == NULL || ident_t->value == NULL)
        return NULL;
    char* ident = xstrdup(ident_t->value);

    struct Node* node;
    if (check(TT_SEMICOLON)) {
        node = init_node(NODE_VAR_DECL, begin);
        node->var_decl.ident = ident;
    }
    else {
        // EQ
        if (expect(TT_EQ) == NULL)
            return NULL;
        node = init_node(NODE_VAR_DEF, begin);
        node->var_def.ident = ident;

        // expr
        node->var_def.expr = parse_expr();
        if (node->var_def.expr == NULL)
            return NULL;
    }

    return node;
}

static struct Node* parse_node_var_assign()
{
    assert(tok_head->type == TT_IDENT);

    if (tok_head->value == NULL)
        return NULL;

    struct Node* node = init_node(NODE_VAR_ASSIGN, tok_head->begin);

    // ident
    node->var_assign.ident = xstrdup(tok_head->value);
    advance();

    // =
    if (expect(TT_EQ) == NULL)
        return NULL;

    // expr
    node->var_assign.expr = parse_expr();
    if (node->var_assign.expr == NULL)
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

    return node;
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

struct Node* parse(const struct Token* tokens_head, struct ErrorList* errorlist)
{
    tok_head = tokens_head;
    errors = errorlist;

    // Todo: parse block better ,with '{' and '}'
    struct Node* n_block = init_node(NODE_BLOCK, tok_head->begin);
    init_nodelist(&n_block->block.nodes, 10);

    while (tok_head->type != TT_END) {
        struct Node* cur_node = NULL;
        switch (tok_head->type) {
            case TT_VAR:
                cur_node = parse_node_var_decl_def();
                break;

            case TT_IDENT:
                cur_node = parse_node_var_assign();
                break;

            case TT_RETURN:
                cur_node = parse_node_return();
                break;

            case TT_AT:
                cur_node = parse_node_builtin();
                break;

            default:
                const struct Token* t = advance();
                add_error(errors, ERROR_SYNTAX, t->begin, "Unexpected token: '%s'", tt_str(t->type));
        }
        // NOTE: order matters a || b
        if (cur_node == NULL || !expect(TT_SEMICOLON)) {
            // Skip to next statement or to end
            skip_statement();
            continue;
        }
        nodelist_push(&n_block->block.nodes, cur_node);
    }
    return n_block;
}

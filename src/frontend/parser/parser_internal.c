#include "frontend/parser/parser_internal.h"

#include <assert.h>

#include "utils/xalloc.h"


static struct Parser* ctx = NULL;

static bool is_sync_token()
{
    return check(TT_END) || check(TT_SEMICOLON) || check(TT_RBRACE)
        || check(TT_COMMA)
        || check(TT_LBRACE) || check(TT_RPAREN) || check(TT_LPAREN)
        || check(TT_AT) || check(TT_IDENT) || check(TT_RETURN)
        || check(TT_VAR) || check(TT_FUNC);
}

void init_parser(struct Parser* p, const struct Token* tok_head, struct ErrorList* errorlist)
{
    p->tok_head = tok_head;
    p->errors = errorlist;
    p->out_of_sync = false;
}

void set_ctx(struct Parser* p)
{
    ctx = p;
}

void unset_ctx()
{
    ctx = NULL;
}

const struct Token* peek()
{
    assert(ctx);
    return ctx->tok_head;
}

struct ErrorList* errors_ptr()
{
    assert(ctx);
    return ctx->errors;
}

const struct Token* advance()
{
    assert(ctx);
    const struct Token* last = peek();
    if (last->type != TT_END)
        ctx->tok_head = last->next;

    return last;
}

bool check(enum TokenType tt)
{
    assert(ctx);
    const struct Token* t = peek();
    return t->type == tt; 
}

bool check_next(enum TokenType tt)
{
    assert(ctx);
    const struct Token* t = peek()->next;

    if (t == NULL) {
        return false;
    }
    return t->type == tt;
}

const struct Token* expect(enum TokenType tt)
{
    assert(ctx);
    const struct Token* t = peek();
    if (!check(tt)) {
        add_error(
            ctx->errors, ERROR_SYNTAX, t->begin,
            "Expected: '%s', Actual: '%s'",
            tt_str(tt), tt_str(t->type)
        );
        return NULL;
    }
    return advance();
}

void set_need_sync()
{
    ctx->out_of_sync = true;
}

bool need_sync()
{
    return !ctx || ctx->out_of_sync;
}

void sync()
{
    while (!is_sync_token() && peek()->next != NULL) {
        advance();
    }
    ctx->out_of_sync = false;
}
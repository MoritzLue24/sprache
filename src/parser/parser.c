// The parser turns the flat token list from the lexer into the tree
// described in "parser/ast.h". It is a hand-written recursive
// parser; only the binary operators use precedence climbing, because
// writing one function per precedence level does not scale with a table
// that grows by one line per operator.
//
//
// LAYOUT
//
// struct Parser, the recovery levels, then forward declarations for every
// static function in the order they are defined below. The definitions
// follow the grammar top down: parse() -> func_def -> block -> stmt ->
// expr -> binary -> unary -> factor, so reading the file straight
// through reads the grammar. The cursor primitives, error() and sync()
// sit at the bottom, since they are machinery, not grammar.
//
//
// TABLES VS SWITCHES
//
// Precedence and associativity live in "parser/binop.def", the prefix
// operators in "parser/unop.def". parse_binary() and parse_unary() only
// ask op_kind_binary_prec() / _assoc() / _from_token(), they contain no
// operator names at all. Adding an operator is one line in a .def file;
// touching this file means the *mechanics* changed, not the operator set.
//
//
// CURSOR
//
// peek(), check(), advance() and expect() are the only functions that
// read or move p->i. advance() clamps on the last token, and the lexer
// always appends TK_END, so the cursor can never run off the list and
// every loop can rely on seeing TK_END forever once it gets there.
//
//
// ERROR RECOVERY
//
// Rests on two separate mechanisms.
//
//   p->panic  suppresses REPORTING, so one broken construct yields one
//             diagnostic instead of a dozen. Only error() sets it to
//             true; only sync() and the collecting loops clear it.
//   sync()    moves the CURSOR to a token where parsing can resume. Only
//             the collecting loops call it. parse_func_def(), parse_stmt()
//             and everything below them report and return NULL, and leave
//             the decision where to resume to their caller, otherwise
//             two levels would fight over the same cursor.
//
// A collecting loop clears p->panic at the top of each iteration: the
// first token of a fresh item is solid ground, and whatever went wrong in
// the previous item must not silence the next one.
//
// sync() distinguishes two kinds of tokens. A TERMINATOR closes the broken
// unit (the ';' of a statement) and is consumed. A STOP token opens the
// next one ('fn', 'var', '}') and is left for the caller, which is about
// to dispatch on it. TK_FUNC stops every level, it is the strongest
// anchor the language has.
//
//
// TWO INVARIANTS
//
// A node-returning function returns NULL only with p->panic set. That
// lets the loops test p->panic instead of the return value, which also
// covers a node that survived while its separator did not.
//
// Every collecting loop consumes at least one token per iteration. This
// is the only thing standing between a malformed input and an endless
// loop, so each loop below carries the argument for why it holds.
//
//
// WHEN A NODE SURVIVES
//
// An error that leaves the meaning of a construct open discards its node.
// An error that only concerns a separator is reported, and the node
// stays -- "a = 1 b = 2" is two understood statements and one missing
// semicolon, and later stages are better served by the two statements
// than by a hole.

#include "parser/parser.h"
#include "utils/arena.h"
#include "sprache/diag.h"
#include "utils/darray.h"
#include "diag/diag_internal.h"
#include <stddef.h>
#include <assert.h>
#include <stdarg.h>

struct Parser {
    struct Arena* a;
    const struct TokenList* tkl;
    struct DiagList* dl;
    size_t i;
    bool panic;
};

// Recovery levels. Each one names a place the parser knows how to pick up
// again: a fresh top-level item, a fresh statement, the next parameter,
// the next argument.
enum SyncLevel {
    SYNC_TOPLEVEL,
    SYNC_STATEMENT,
    SYNC_PARAM,
    SYNC_ARG,
};

static struct Node* parse_func_def(struct Parser* p);
static struct NodeList parse_params(struct Parser* p);
/// @brief Parses a NODE_TYPE, consumes an TK_IDENT 
static struct Node* parse_type(struct Parser* p);
static struct Node* parse_block(struct Parser* p);
static struct Node* parse_stmt(struct Parser* p);
static struct Node* parse_var_decl_or_def(struct Parser* p);
static struct Node* parse_return(struct Parser* p);
static struct Node* parse_expr(struct Parser* p);
static struct Node* parse_binary(struct Parser* p, int min_prec);
static struct Node* parse_unary(struct Parser* p);
static struct Node* parse_factor(struct Parser* p);
static struct Node* parse_ident_or_call(struct Parser* p);
static struct Node* parse_builtin(struct Parser* p);
static struct NodeList parse_args(struct Parser* p);

static struct Token peek(const struct Parser* p);
static bool check(const struct Parser* p, enum TokenKind tk);
static struct Token advance(struct Parser* p);
static struct Token expect(struct Parser* p, enum TokenKind tk);

static void error(struct Parser* p, enum DiagCode code, ...);
static void sync(struct Parser* p, enum SyncLevel lvl);
/// @brief A terminator belongs to the broken unit, consume it and stop.
static bool sync_is_terminator(enum SyncLevel lvl, enum TokenKind tk);
/// @brief A stop token starts the next unit, leave it for the caller.
static bool sync_is_stop(enum SyncLevel lvl, enum TokenKind tk);

struct Node parse(
    struct Arena* a, const struct TokenList* tkl, struct DiagList* dl
) {
    // peek() and advance() index the last token unconditionally; the lexer
    // always appends TK_END, so the list is never empty.
    assert(tkl->count > 0);

    struct Parser p = { .a = a, .tkl = tkl, .dl = dl, .i = 0 };
    struct Node n_program;

    node_init(&n_program, NODE_PROGRAM, SOURCE_LOC_NULL, OP_INVALID, NULL);
    DARRAY_INIT(a, &n_program.program.nl, NODELIST_INIT_CAPACITY);

    while (!check(&p, TK_END)) {
        switch (peek(&p).kind) {
            case TK_FUNC: {
                // Solid ground: a fresh item starts here, so diagnostics
                // are allowed again.
                p.panic = false;

                struct Node* n = parse_func_def(&p);
                if (n != NULL) DARRAY_ADD(a, &n_program.program.nl, n);
                break;
            }
            default:
                error(
                    &p, DIAG_UNEXPECTED_TOKEN, token_kind_str(peek(&p).kind)
                );
                break;
        }
        // Progress: in the default branch the cursor sits on a token that
        // is not TK_FUNC, so sync() moves at least once. In the TK_FUNC
        // branch parse_func_def() has already eaten the 'fn' it was
        // dispatched on, so sync() may stop on the next 'fn' for free.
        if (p.panic) sync(&p, SYNC_TOPLEVEL);
    }
    return n_program;
}

static struct Node* parse_func_def(struct Parser* p)
{
    struct Token tk_kw = expect(p, TK_FUNC);
    if (tk_kw.kind == TK_INVALID) return NULL;

    struct Token tk_ident = expect(p, TK_IDENT);
    if (tk_ident.kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_FUNC_DEF, tk_kw.loc, OP_INVALID, tk_ident.value);

    if (expect(p, TK_LPAREN).kind == TK_INVALID) return NULL;
    n->func_def.params = parse_params(p);
    if (expect(p, TK_RPAREN).kind == TK_INVALID) return NULL;

    if (expect(p, TK_ARROW).kind == TK_INVALID) return NULL;
    n->func_def.type = parse_type(p);
    if (n->func_def.type == NULL) return NULL;

    n->func_def.body = parse_block(p);
    // A function without a usable body is not worth keeping: propagating
    // NULL sends parse() into sync(SYNC_TOPLEVEL), which skips to the next
    // 'fn'. This function never calls sync() itself, resuming is the
    // caller's decision.
    if (n->func_def.body == NULL) return NULL;

    return n;
}

static struct NodeList parse_params(struct Parser* p)
{
    struct NodeList params;
    DARRAY_INIT(p->a, &params, NODELIST_INIT_CAPACITY);

    if (check(p, TK_RPAREN)) return params;

    do {
        // Solid ground: a fresh parameter starts here.
        p->panic = false;

        struct Token tk_ident = expect(p, TK_IDENT);
        if (tk_ident.kind != TK_INVALID) {
            struct Node* n_param = ARENA_CALLOC(p->a, struct Node);
            node_init(
                n_param, NODE_PARAM, tk_ident.loc, OP_INVALID, tk_ident.value
            );
            if (expect(p, TK_COLON).kind != TK_INVALID) {
                n_param->param.type = parse_type(p);
                if (n_param->param.type != NULL) {
                    DARRAY_ADD(p->a, &params, n_param);
                }
            }
        }
        // SYNC_PARAM stops BEFORE the comma, because the comma belongs to
        // the loop below. Progress: if the cursor already sits on one,
        // sync() returns without consuming and the advance() below moves;
        // on ')' or '{' the loop breaks; otherwise sync() itself moves.
        if (p->panic) sync(p, SYNC_PARAM);

        if (check(p, TK_COMMA)) advance(p);
        else break;
    } while (true);

    return params;
}

static struct Node* parse_type(struct Parser* p)
{
    struct Token tk_type = expect(p, TK_IDENT);
    if (tk_type.kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_TYPE, tk_type.loc, OP_INVALID, tk_type.value);
    return n;
}

static struct Node* parse_block(struct Parser* p)
{
    struct Token tk_lbrace = expect(p, TK_LBRACE);
    if (tk_lbrace.kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_BLOCK, tk_lbrace.loc, OP_INVALID, NULL);
    DARRAY_INIT(p->a, &n->block.nl, NODELIST_INIT_CAPACITY);

    // The condition must exit on every SYNC_STATEMENT stop token that
    // parse_stmt() does not consume by itself: TK_RBRACE, TK_FUNC,
    // TK_END. TK_LBRACE / TK_VAR / TK_RETURN need no exit here,
    // because parse_stmt()'s dispatch always eats them. Get this
    // wrong and sync() returns without consuming while the loop
    // retries the same token.
    while (
        !check(p, TK_RBRACE) && !check(p, TK_FUNC) && !check(p, TK_END)
    ) {
        // Solid ground: a fresh statement starts here.
        p->panic = false;

        struct Node* stmt = parse_stmt(p);
        if (stmt != NULL) DARRAY_ADD(p->a, &n->block.nl, stmt);

        // Syncing on p->panic rather than on "stmt == NULL" also covers a
        // node that survived while only its separator was broken.
        // Progress: '}', 'fn' and TK_END end the loop; ';' is consumed by
        // sync() itself; '{', 'var' and 'return' are eaten unconditionally
        // by parse_stmt()'s dispatch; everything else sync() walks past.
        if (p->panic) sync(p, SYNC_STATEMENT);
    }

    if (expect(p, TK_RBRACE).kind == TK_INVALID) return NULL;
    return n;
}

static struct Node* parse_stmt(struct Parser* p)
{
    switch (peek(p).kind) {
        case TK_LBRACE: return parse_block(p);
        case TK_VAR: return parse_var_decl_or_def(p);
        case TK_RETURN: return parse_return(p);
        default: {
            struct Node* n = parse_expr(p);
            if (n == NULL) return NULL;

            // A missing separator does not put the meaning of the
            // expression in doubt, so the node stays. expect() has set
            // p->panic, which sends the caller's loop into sync().
            expect(p, TK_SEMICOLON);
            return n;
        }
    }
    return NULL;
}

static struct Node* parse_var_decl_or_def(struct Parser* p)
{
    struct Token tk_var = expect(p, TK_VAR);
    if (tk_var.kind == TK_INVALID) return NULL;

    struct Token tk_ident = expect(p, TK_IDENT);
    if (tk_ident.kind == TK_INVALID) return NULL;

    if (expect(p, TK_COLON).kind == TK_INVALID) return NULL;
    struct Node* n_type = parse_type(p);
    if (n_type == NULL) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);

    if (!check(p, TK_SEMICOLON)) {
        node_init(n, NODE_VAR_DEF, tk_var.loc, OP_INVALID, tk_ident.value);
        if (expect(p, TK_EQ).kind == TK_INVALID) return NULL;
        struct Node* expr = parse_expr(p);
        if (expr == NULL) return NULL;
        n->var_def.type = n_type;
        n->var_def.init = expr;
    }
    else {
        node_init(n, NODE_VAR_DECL, tk_var.loc, OP_INVALID, tk_ident.value);
        n->var_decl.type = n_type;
    }
    // Separator error: report and keep the node (see parse_stmt).
    expect(p, TK_SEMICOLON);
    return n;
}

static struct Node* parse_return(struct Parser* p)
{
    struct Token tk_return = expect(p, TK_RETURN);
    if (tk_return.kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_RETURN, tk_return.loc, OP_INVALID, NULL);

    if (!check(p, TK_SEMICOLON)) {
        n->ret.expr = parse_expr(p);
        if (n->ret.expr == NULL) return NULL;
    }
    else {
        n->ret.expr = NULL;
    }

    expect(p, TK_SEMICOLON);
    return n;
}

static struct Node* parse_expr(struct Parser* p)
{
    return parse_binary(p, 1);
}

static struct Node* parse_binary(struct Parser* p, int min_prec)
{
    struct Node* lhs = parse_unary(p);
    if (lhs == NULL) return NULL;

    for (;;) {
        enum OpKind op = op_kind_binary_from_token(peek(p).kind);
        if (op == OP_INVALID || op_kind_binary_prec(op) < min_prec) break;

        struct SourceLoc loc = advance(p).loc;
        int next = op_kind_binary_prec(op);
        if (op_kind_binary_assoc(op) == ASSOC_LEFT) next++;

        struct Node* rhs = parse_binary(p, next);
        if (rhs == NULL) return NULL;

        struct Node* n = ARENA_CALLOC(p->a, struct Node);
        node_init(n, NODE_BINARY, loc, op, NULL);
        n->binary.lhs = lhs;
        n->binary.rhs = rhs;
        lhs = n;
    }

    return lhs;
}

static struct Node* parse_unary(struct Parser* p)
{
    enum OpKind op = op_kind_unary_from_token(peek(p).kind);
    if (op == OP_INVALID) return parse_factor(p);

    struct SourceLoc loc = advance(p).loc;
    struct Node* operand = parse_unary(p);
    if (operand == NULL) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_UNARY, loc, op, NULL);
    n->unary.operand = operand;
    return n;
}

static struct Node* parse_factor(struct Parser* p)
{
    if (check(p, TK_LITERAL)) {
        struct Token tk = advance(p);
        struct Node* n = ARENA_CALLOC(p->a, struct Node);
        node_init(n, NODE_LITERAL, tk.loc, OP_INVALID, tk.value);
        return n;
    }
    else if (check(p, TK_IDENT)) {
        return parse_ident_or_call(p);
    }
    else if (check(p, TK_AT)) {
        return parse_builtin(p);
    }
    else if (check(p, TK_LPAREN)) {
        struct Token lparen = advance(p);
        struct Node* expr = parse_expr(p);
        if (expr == NULL) return NULL;
        if (expect(p, TK_RPAREN).kind == TK_INVALID) return NULL;

        expr->loc = lparen.loc;
        return expr;
    }
    // The cursor stays put on purpose: dropping the offending token is
    // recovery, and recovery belongs to the collecting loops.
    error(p, DIAG_EXPRESSION_EXPECTED, token_kind_str(peek(p).kind));
    return NULL;
}

static struct Node* parse_ident_or_call(struct Parser* p)
{
    struct Token tk_ident = expect(p, TK_IDENT);
    if (tk_ident.kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    if (!check(p, TK_LPAREN)) {
        node_init(n, NODE_IDENT, tk_ident.loc, OP_INVALID, tk_ident.value);
        return n;
    }
    advance(p);
    struct NodeList args = parse_args(p);
    if (expect(p, TK_RPAREN).kind == TK_INVALID) return NULL;

    node_init(n, NODE_CALL, tk_ident.loc, OP_INVALID, tk_ident.value);
    n->call.args = args;
    return n;
}

static struct Node* parse_builtin(struct Parser* p)
{
    if (expect(p, TK_AT).kind == TK_INVALID) return NULL;
    struct Token tk_ident = expect(p, TK_IDENT);
    if (tk_ident.kind == TK_INVALID) return NULL;

    if (expect(p, TK_LPAREN).kind == TK_INVALID) return NULL;
    struct NodeList args = parse_args(p);
    if (expect(p, TK_RPAREN).kind == TK_INVALID) return NULL;

    struct Node* n = ARENA_CALLOC(p->a, struct Node);
    node_init(n, NODE_BUILTIN, tk_ident.loc, OP_INVALID, tk_ident.value);
    n->builtin.args = args;
    return n;
}

static struct NodeList parse_args(struct Parser* p)
{
    struct NodeList args;
    DARRAY_INIT(p->a, &args, NODELIST_INIT_CAPACITY);

    if (!check(p, TK_RPAREN)) {
        while (true) {
            // Solid ground: a fresh argument starts here.
            p->panic = false;

            struct Node* arg = parse_expr(p);
            if (arg != NULL) DARRAY_ADD(p->a, &args, arg);
            if (p->panic) sync(p, SYNC_ARG);

            if (!check(p, TK_COMMA))
                break;
            advance(p);  // ,
        }
    }
    return args;
}

static struct Token peek(const struct Parser* p)
{
    if (p->i >= p->tkl->count) return p->tkl->items[p->tkl->count - 1];
    return p->tkl->items[p->i];
}

static bool check(const struct Parser* p, enum TokenKind tk)
{
    return peek(p).kind == tk;
}

static struct Token advance(struct Parser* p)
{
    struct Token tk = peek(p);
    if (p->i < p->tkl->count - 1) p->i++;
    return tk;
}

static struct Token expect(struct Parser* p, enum TokenKind tk)
{
    if (check(p, tk)) return advance(p);
    error(
        p, DIAG_DIFFERENT_TOKEN_EXPECTED, token_kind_str(tk),
        token_kind_str(peek(p).kind)
    );
    return TOKEN_NULL;
}

static void error(struct Parser* p, enum DiagCode code, ...)
{
    if (p->panic) return;
    p->panic = true;

    // forwards to diag_vadd, not diag_add. C cannot pass '...' on to
    // another variadic function, so the arguments travel as a va_list. The
    // format itself stays bound to the code in "sprache/diag.def".
    va_list args;
    va_start(args, code);
    diag_vadd(p->a, p->dl, code, peek(p).loc, args);
    va_end(args);
}

static void sync(struct Parser* p, enum SyncLevel lvl)
{
    p->panic = false;

    // TK_END has to end this loop explicitly: advance() clamps at the last
    // token, so without the guard sync() would spin there forever.
    while (!check(p, TK_END)) {
        if (sync_is_terminator(lvl, peek(p).kind)) {
            advance(p);
            return;
        }
        if (sync_is_stop(lvl, peek(p).kind)) return;
        advance(p);
    }
}

static bool sync_is_terminator(enum SyncLevel lvl, enum TokenKind tk)
{
    switch (lvl) {
        case SYNC_TOPLEVEL:  return false;
        case SYNC_STATEMENT: return tk == TK_SEMICOLON;
        case SYNC_PARAM:     return false;
        case SYNC_ARG:       return false;
    }
    return false;
}

static bool sync_is_stop(enum SyncLevel lvl, enum TokenKind tk)
{
    switch (lvl) {
        case SYNC_TOPLEVEL:
            return tk == TK_FUNC;
        case SYNC_STATEMENT:
            return tk == TK_RBRACE || tk == TK_LBRACE || tk == TK_VAR
                || tk == TK_RETURN || tk == TK_FUNC;
        case SYNC_PARAM:
            return tk == TK_COMMA || tk == TK_RPAREN || tk == TK_LBRACE
                || tk == TK_SEMICOLON || tk == TK_FUNC;
        case SYNC_ARG:
            return tk == TK_COMMA || tk == TK_RPAREN || tk == TK_SEMICOLON
                || tk == TK_RBRACE || tk == TK_FUNC;
    }
    return false;
}

#ifndef AST_H
#define AST_H

#include "sprache/diag.h"
#include "lexer/tokens.h"
#include <stdio.h>

#define NODELIST_INIT_CAPACITY 20

enum NodeKind {
    NODE_INVALID,
#define NODE(kind) kind,
#include "parser/node.def"
#undef NODE
};

const char* node_kind_str(enum NodeKind nk);

enum AssocKind {
    ASSOC_INVALID,
    ASSOC_LEFT,
    ASSOC_RIGHT
};

enum OpKind {
    OP_INVALID,
#define BINOP(kind, tok, prec, assoc) kind,
#include "parser/binop.def"
#undef BINOP
#define UNOP(kind, tok) kind,
#include "parser/unop.def"
#undef UNOP
};

int op_kind_binary_prec(enum OpKind op);
enum AssocKind op_kind_binary_assoc(enum OpKind op);
const char* op_kind_str(enum OpKind op);
enum OpKind op_kind_binary_from_token(enum TokenKind tk);
enum OpKind op_kind_unary_from_token(enum TokenKind tk);

struct Node;

struct NodeList {
    struct Node** items;
    size_t count;
    size_t capacity;
};

struct Node {
    enum NodeKind kind;
    struct SourceLoc loc;
    enum OpKind op;
    const char* value;

    union {
        struct { struct NodeList nl; } program;
        struct { struct NodeList params; struct Node* body; } func_def;
        struct { struct NodeList nl; } block;
        struct { struct Node* init; } var;
        struct { struct Node* expr; } ret;
        struct { struct Node* lhs; struct Node* rhs; } binary;
        struct { struct Node* operand; } unary;
        struct { struct NodeList args; } call;
        struct { struct NodeList args; } builtin;
    };
};

void node_init(
    struct Node* n, enum NodeKind kind, struct SourceLoc loc, enum OpKind op,
    const char* value
);
void node_dump(const struct Node* n, FILE* out);
void node_dump_all(const struct NodeList* nl, FILE* out);

#endif
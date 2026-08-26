#ifndef AST_H
#define AST_H

#include "sprache/diag.h"
#include "lexer/tokens.h"
#include "type/type.h"
#include <stdio.h>

#define NODELIST_INIT_CAPACITY 20

enum NodeKind {
    NODE_INVALID,
#define NODE(kind) kind,
#include "parser/node.def"
#undef NODE
};

/// @returns the node kind as a string.
/// On invalid kind (not NODE_INVALID), returns NULL.
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

/// @returns the precedence of a binary operator.
/// When the op kind is not binary, return -1.
int op_kind_binary_prec(enum OpKind op);
/// @returns the associativity of a binary operator.
/// When the op kind is not binary, return ASSOC_INVALID.
enum AssocKind op_kind_binary_assoc(enum OpKind op);
/// @returns the op kind as a string.
/// On invalid kind (not OP_INVALID), returns NULL.
const char* op_kind_str(enum OpKind op);
/// @returns the first binary operator that matches the token.
/// On no match, return OP_INVALID.
enum OpKind op_kind_binary_from_token(enum TokenKind tk);
/// @returns the first unary operator that matches the token.
/// On no match, return OP_INVALID.
enum OpKind op_kind_unary_from_token(enum TokenKind tk);

struct Node;

struct NodeList {
    struct Node** items;
    size_t count;
    size_t capacity;
};

struct Symbol;

struct Node {
    enum NodeKind kind;
    struct SourceLoc loc;
    enum OpKind op;
    const char* value;

    /// @brief set by sema.c. TYPE_INVALID until it has run.
    enum TypeKind type;
    /// @brief set by sema.c. NULL until it has run.
    /// Also stores information about builtins in symbol.builtin_kind
    struct Symbol* symbol;

    union {
        struct { struct NodeList nl; } program;
        struct {
            struct Node* type;
            struct NodeList params;
            struct Node* body;
        } func_def;
        struct { struct Node* type; } param;
        struct { struct NodeList nl; } block;
        struct { struct Node* type; } var_decl;
        struct { struct Node* type; struct Node* init; } var_def;
        struct { struct Node* expr; } ret;
        struct { struct Node* lhs; struct Node* rhs; } binary;
        struct { struct Node* operand; } unary;
        struct { struct NodeList args; } call;
        struct { struct NodeList args; } builtin;
    };
};

/// @brief Initialises the given node with the supplied fields.
/// Every other field is set to zero.
void node_init(
    struct Node* n, enum NodeKind kind, struct SourceLoc loc, enum OpKind op,
    const char* value
);
void node_dump(const struct Node* n, FILE* out);
void node_dump_all(const struct NodeList* nl, FILE* out);

#endif
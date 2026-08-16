#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "core/loc.h"
#include "core/type.h"
#include "frontend/tokenizer/tokens.h"

#define NODELIST_INIT_CAPACITY 10


enum NodeType {
    NODE_INVALID,

    // top-level
    NODE_PROGRAM,
    NODE_FUNC_DEF,
    NODE_PARAM,

    // statements
    NODE_BLOCK,
    NODE_VAR_DECL,  // var a;
    NODE_VAR_DEF,   // var a = 2;
    NODE_RETURN,

    // expr
    NODE_ASSIGN_EXPR,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_VAR,
    NODE_CALL,
    NODE_LITERAL,

    // builtins
    NODE_BUILTIN
};

enum OpType {
    OP_INVALID,

    OP_PLUS,
    OP_MINUS,
    OP_MUL,

    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,

    OP_BW_NOT,
    OP_BW_AND,
    OP_BW_OR,
    OP_BW_XOR
};


// forward decl
struct Symbol;
struct Builtin;

struct Node {
    enum NodeType type;
    struct Loc begin;
    struct Node* next;

    union {
        struct {
            struct Node* items_head;
        } program;

        struct {
            char* ident;
            struct Node* params_head;
            enum Type ret_type;
            struct Node* body;
            const struct Symbol* symbol;
        } func_def;

        struct {
            char* ident;
            enum Type typekind;
            const struct Symbol* symbol;
        } param;

        struct {
            struct Node* statements_head;
        } block;

        struct {
            // type: NODE_VAR
            struct Node* target;
            enum Type typekind;
        } var_decl;

        struct {
            // type: NODE_VAR
            struct Node* target;
            enum Type typekind;
            struct Node* expr;
        } var_def;

        /// @brief ret = return
        struct {
            struct Node* expr;
        } ret;

        struct {
            struct Node* target;
            struct Node* expr;
        } assign_expr;

        struct {
            enum OpType op;
            struct Node* factor;
        } unary_op;

        struct {
            enum OpType op;
            struct Node* lhs;
            struct Node* rhs;
        } bin_op;

        struct {
            char* ident;
            /// @brief semantic annotation
            const struct Symbol* symbol;
        } var;

        struct {
            char* ident;
            const struct Symbol* symbol;
            struct Node* args_head;
        } call;

        struct {
            char* value;
        } literal;

        struct {
            char* ident;
            const struct BuiltinDef* def;
            struct Node* args_head;
        } builtin;
    };
};

struct Node* alloc_node(enum NodeType type, struct Loc begin);

struct Node* alloc_invalid_node();

struct Node* push_node(struct Node* head, struct Node* node);

size_t nodelist_size(const struct Node* head);

enum OpType tt_to_op(enum TokenType tt);

const char* op_type_str(enum OpType type);

/// @brief prints the ast of the given node with the given depth as indentation, and with a label in front (NULL for nothing)
void print_node(const char* label, const struct Node* node, int depth);

void free_node(struct Node* node);

#endif
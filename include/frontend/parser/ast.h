#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "frontend/core/loc.h"
#include "frontend/tokenizer/tokens.h"


enum NodeType {
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
    NODE_BINARY_OP,
    NODE_VAR,
    NODE_CALL,
    NODE_LITERAL,

    // builtins
    NODE_BUILTIN
};

enum OpType {
    OP_PLUS,
    OP_MINUS,
    OP_MUL
};

struct NodeList {
    struct Node** data;
    size_t size;
    size_t capacity;
    size_t head;
};

// forward decl
struct Symbol;
struct Builtin;

struct Node {
    enum NodeType type;
    struct Loc begin;

    union {
        struct {
            struct NodeList items;
        } program;

        struct {
            char* ident;
            const struct Symbol* symbol;
            struct NodeList params;
            struct Node* body;
        } func_def;

        struct {
            char* ident;
            const struct Symbol* symbol;
        } param;

        struct {
            struct NodeList statements;
        } block;

        struct {
            // type: NODE_VAR
            struct Node* target;
        } var_decl;

        struct {
            // type: NODE_VAR
            struct Node* target;
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
            struct NodeList args;
        } call;

        struct {
            char* value;
        } literal;

        struct {
            char* ident;
            const struct BuiltinDef* def;
            struct NodeList args;
        } builtin;
    };
};

void init_nodelist(struct NodeList* nl, size_t capacity);

bool nodelist_push(struct NodeList* nl, struct Node* node);

void free_nodelist(struct NodeList* nl);

const char* op_type_str(enum OpType type);

/// @brief prints the ast of the given node with the given depth as indentation, and with a label in front (NULL for nothing)
void print_node(const char* label, const struct Node* node, int depth);

void free_node(struct Node* node);

#endif
#ifndef AST_H
#define AST_H

#include <stdlib.h>

#include "core/loc.h"
#include "parser/tokens.h"
#include "sema/symbols.h"
#include "sema/builtins.h"


enum NodeType {
    NODE_BLOCK,
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_VAR_DECL,  // var a;
    NODE_VAR_DEF,   // var a = 2;
    NODE_VAR_ASSIGN,    // a = 2;
    NODE_VAR,   // a
    NODE_RETURN,
    NODE_BUILTIN
};

enum OpType {
    OP_PLUS,
    OP_MINUS,
    OP_MUL
};

/// @brief A dynamic array of nodes, optimized for
/// first pushing, then popping (FIFO).
/// You HAVE to first push everything, then pop. No interference
struct NodeList {
    struct Node** data;
    size_t size;
    size_t capacity;
    size_t head;
};

struct Node {
    enum NodeType type;
    struct Loc begin;

    union {
        struct {
            struct NodeList nodes;
        } block;

        struct {
            char* ident;
            /// @brief semantic annotation
            const struct Symbol* symbol;
        } var_decl;

        struct {
            char* ident;
            struct Node* expr;
            /// @brief semantic annotation
            const struct Symbol* symbol;
        } var_def;

        struct {
            char* ident;
            struct Node* expr;
            /// @brief semantic annotation
            const struct Symbol* symbol;
        } var_assign;

        struct {
            char* ident;
            /// @brief semantic annotation
            const struct Symbol* symbol;
        } var;

        /// @brief ret = return
        struct {
            struct Node* expr;
        } ret;

        struct {
            enum OpType op;
            struct Node* lhs;
            struct Node* rhs;
        } bin_op;

        struct {
            char* value;
        } literal;

        struct {
            char* ident;
            struct NodeList args;
            /// @brief semantic annotation
            const struct BuiltinDef* def;
        } builtin;
    };
};

void init_nodelist(struct NodeList* nl, size_t capacity);

bool nodelist_push(struct NodeList* nl, struct Node* node);

/// @brief Pops the FIRST added element.
/// IMPORTANT: ownership is transfered to the caller.
/// Freeing the Node over nodelist is not longer possible
struct Node* nodelist_pop_first(struct NodeList* nl);

struct Node* nodelist_get(const struct NodeList* nl, size_t i);

void free_nodelist(struct NodeList* nl);

const char* op_type_str(enum OpType type);

/// @brief prints the ast of the given node with the given depth as indentation, and with a label in front (NULL for nothing)
void print_node(const char* label, const struct Node* node, int depth);

void free_ast(struct Node* ast);

#endif
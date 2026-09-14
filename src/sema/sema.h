#ifndef SEMA_H
#define SEMA_H

#include <stdbool.h>

struct Arena;
struct Node;
struct DiagList;

/// @brief Runs a semantic analysis on the provided AST, creating a symbol table,
/// annotating the Node.type & node.symbol fields. Errors are added to dl.
/// @note Asserts when the provided Node is not a valid NODE_PROGRAM, or
/// the AST is not valid on other aspects.
// @claude-begin
/// @note Must only run on a tree that parsed without errors, a broken tree
/// produces cascades of bogus diagnostics.
// @claude-end
void sema_check(struct Arena* a, struct Node* n_program, struct DiagList* dl);

#endif
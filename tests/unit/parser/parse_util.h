#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "diag/diag_internal.h"
#include "utils/darray.h"

/// @brief One parse run: arena, collected diagnostics and the program node.
/// @note Everything the parser allocates lives in 'a', so parsed_free()
///       releases the whole run at once.
struct Parsed {
    struct Arena a;
    struct DiagList dl;
    struct Node root;
};

static inline void parsed_init(struct Parsed* p, const char* source)
{
    arena_init(&p->a);
    DARRAY_INIT(&p->a, &p->dl, DIAG_INIT_CAPACITY);

    struct TokenList tkl = lex(&p->a, source);
    p->root = parse(&p->a, &tkl, &p->dl);
}

static inline void parsed_free(struct Parsed* p)
{
    arena_free(&p->a);
}

// Stand-in for a node the parser did not produce. NODE_INVALID is the
// first enumerator, so "{0}" yields exactly that, with every list empty
// and every child NULL. Navigating past a missing node therefore keeps
// yielding this node instead of dereferencing NULL, and a failing test
// reports a kind mismatch instead of taking the whole run down.
static const struct Node node_none = { 0 };

/// @brief The i-th entry of 'nl', or node_none if the list is shorter.
static inline const struct Node* at(const struct NodeList* nl, size_t i)
{
    if (nl == NULL || i >= nl->count || nl->items[i] == NULL)
        return &node_none;
    return nl->items[i];
}

/// @brief A single child, or node_none if the parser left it NULL.
static inline const struct Node* some(const struct Node* n)
{
    return n == NULL ? &node_none : n;
}

/// @brief The value string, or "" -- keeps strcmp out of NULL territory.
static inline const char* val(const struct Node* n)
{
    return n->value == NULL ? "" : n->value;
}

/// @brief Body of the i-th top-level function.
static inline const struct Node* fn_body(const struct Node* root, size_t i)
{
    const struct Node* fn = at(&root->program.nl, i);
    if (fn->kind != NODE_FUNC_DEF) return &node_none;
    return some(fn->func_def.body);
}

/// @brief The j-th statement in the body of the i-th top-level function.
static inline const struct Node* fn_stmt(
    const struct Node* root, size_t i, size_t j
) {
    return at(&fn_body(root, i)->block.nl, j);
}

#endif

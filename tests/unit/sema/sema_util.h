// @claude-begin
#ifndef SEMA_UTIL_H
#define SEMA_UTIL_H

#include "unit/parser/parse_util.h"
#include "sema/sema.h"
#include "sema/symbols.h"

/// @brief Parses and checks 'source' into 'p'.
/// @note Like sprache_compile, sema only runs if parsing succeeded. A test
///       with a broken input therefore fails on the parser's diagnostic.
static inline void checked_init(struct Parsed* p, const char* source)
{
    parsed_init(p, source);
    if (p->dl.count == 0) sema_check(&p->a, &p->root, &p->dl);
}

/// @brief Code of the i-th diagnostic, or DIAG_INVALID if there are fewer.
static inline enum DiagCode diag_at(const struct Parsed* p, size_t i)
{
    if (i >= p->dl.count) return DIAG_INVALID;
    return p->dl.items[i].code;
}

#endif
// @claude-end

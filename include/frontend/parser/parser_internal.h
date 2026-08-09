#ifndef PARSER_INTERNAL_H

#include "frontend/core/error.h"
#include "frontend/tokenizer/tokens.h"


enum SyncMode {
    SYNC_COMP_STMT,
    SYNC_SIMPLE_STMT,
    SYNC_EXPR,
    SYNC_PARAM
};

struct Parser {
    const struct Token* tok_head;
    struct ErrorList* errors;
    bool out_of_sync;
    enum SyncMode sync_mode;
};


/// @brief Initializes a parser (not dynamically),
/// To set as context, call `set_ctx`
/// @note tok_head needs to end with TT_END to ensure all functionality
void init_parser(struct Parser* p, const struct Token* tok_head, struct ErrorList* errorlist);

/// @brief Sets the current parsing context
/// Asserts, if there already is a context
void set_ctx(struct Parser* p);

/// @brief Unsets the current parsing context
void unset_ctx();

/// @brief Returns a pointer to the current token, should never be null.
/// @note active parser context needed, call `set_ctx`
const struct Token* peek();

/// @brief Returns pointer to the errorlist of current context
struct ErrorList* errors_ptr();

/// @brief Steps one token further, returns
/// the token that was the current token BEFORE the step.
/// @note If the current token is END, do nothing, still return
/// @note active parser context needed, call `set_ctx`
const struct Token* advance();

/// @brief Returns true if the current tokens type matches `tt`
/// @note active parser context needed, call `set_ctx`
bool check(enum TokenType tt);

/// @brief Returns true if the next tokens type matches `tt`.
/// false otherwise, or if next == NULL.
/// @note active parser context needed, call `set_ctx`
bool check_next(enum TokenType tt);

/// @brief If the current token matches `tt`, return this token & advance.
/// Otherwise, return NULL.
/// @note active parser context needed, call `set_ctx`
const struct Token* expect(enum TokenType tt);

void set_need_sync();

bool need_sync();

void sync();

#endif
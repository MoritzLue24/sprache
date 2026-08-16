#include "frontend/tokenizer/lexer.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "utils/xalloc.h"
#include "core/loc.h"


static struct Token* tail = NULL;
static struct Token* head = NULL;
static struct Loc loc;


/// @brief Pushes a Token to the linked list (`tail`, `head`)
static void push(enum TokenType type, char* value, struct Loc begin/* , struct Loc end */)
{
    if (tail == NULL) {
        tail = xmalloc(sizeof(struct Token));
        head = tail;
    }
    else {
        tail->next = xmalloc(sizeof(struct Token));
        tail = tail->next;
    }
    tail->type = type;
    tail->value = value;
    tail->begin = begin;
    // tail->end = end;
    tail->next = NULL;
}

/// @brief Returns the first with the current location matching punctuation
static const struct MatchTypePair* match_punct()
{
    const struct MatchTypePair* match = NULL;
    size_t longest_len = 0;

    for (size_t i = 0; i < punctuations_count; i++) {
        size_t len = strlen(punctuations[i].match);
        if (!strncmp(loc_ptr(&loc), punctuations[i].match, len) && len > longest_len) {
            longest_len = len;
            match = &punctuations[i];
        }
    }
    return match;
}

/// @brief Tokenizes the literal starting at the current loc, pushes the token
///
/// Steps over all directly attached digits, so the `loc` after calling this function
/// is the first char after the literal. Copies the substring of `loc.source`
/// To a new `struct Token`, pushes this token to the linked list
///
/// @note Assumes current loc.c is a digit
static void lex_literal()
{
    assert(isdigit(loc.c));
    const char* ptr = loc_ptr(&loc);
    size_t len = 0;

    // struct Loc begin, end;
    // begin = end = loc;
    struct Loc begin = loc;

    do {
        // end = loc;
        step(&loc);
        len++;
    } while (isdigit(loc.c));

    char* value = xmalloc(len + 1);
    strncpy(value, ptr, len);
    value[len] = '\0';

    push(TT_LITERAL, value, begin);
}

/// @brief Tokenizes the identifier / keyword, starting at the current loc, pushes the token
///
/// Steps over all directly attached alphas, digits, '_', so the `loc`
/// after calling this function is the first char after this identifier / keyword.
/// If a matching keyword exists: Return this keyword as a token. Otherwise, 
/// return ident token with corresponding value (needs freeing).
///
/// @note Assumes current loc.c is alpha, digit or '_'
static void lex_ident()
{
    const char* ptr = loc_ptr(&loc);
    size_t len = 0;
    // struct Loc begin, end;
    // begin = end = loc;
    struct Loc begin = loc;

    do {
        // end = loc;
        step(&loc);
        len++;
    } while (isalnum(loc.c) || loc.c == '_');

    char* value = xmalloc(len + 1);
    strncpy(value, ptr, len);
    value[len] = '\0';

    // keyword check
    bool is_keyword = false;
    for (size_t i = 0; i < keywords_count; i++) {
        if (!strcmp(keywords[i].match, value)) {
            is_keyword = true;
            push(keywords[i].type, NULL, begin);
            break;
        }
    }

    if (is_keyword) {
        xfree((void**)&value);
    }
    // identifier
    else {
        push(TT_IDENT, value, begin);
    }
}


struct Token* lex(const char* source)
{
    head = tail = NULL;
    init_loc(&loc, source);
    while (!loc.end) {
        while (isspace(loc.c))
            step(&loc);

        if (loc.end)
            break;

        if (isdigit(loc.c)) {
            lex_literal();
        }
        else if (isalpha(loc.c) || loc.c == '_') {
            lex_ident();
        }
        else {
            const struct MatchTypePair* punct_match = match_punct();
            if (punct_match != NULL) {
                // struct Loc begin, end;
                // begin = end = loc;
                struct Loc begin = loc;

                for (size_t i = 0; i < strlen(punct_match->match); i++) {
                    step(&loc);
                    // if (i != strlen(punct_match->match) - 1)
                    //     end = loc;
                }
                push(punct_match->type, NULL, begin);
            }
            else {
                struct Loc begin = loc;
                step(&loc);
                push(TT_INVALID, NULL, begin);
            }
        }
    }
    push(TT_END, NULL, loc);
    return head;
}

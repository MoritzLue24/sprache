#include "lexer/lexer.h"
#include "utils/darray.h"
#include <ctype.h>
#include <assert.h>
#include <string.h>

#define TOKENLIST_INIT_CAPACITY 50

/// @brief Advances by exactly one character and returns it.
/// @note Does not skip whitespace. Returns '\0' at end of input without
/// advancing further.
static char loc_step(const char* source, size_t* i, struct SourceLoc* loc);

/// @brief Tokenizes the current literal (assumes the current char is a digit)
static struct Token lex_literal(
    struct Arena* a, const char* source, size_t* i, struct SourceLoc* loc
);

/// @brief Tokenizes the current identifier OR keyword
/// (Assumes the current char is a '_' or alpha)
static struct Token lex_ident_kw(
    struct Arena* a, const char* source, size_t* i, struct SourceLoc* loc
);

/// @brief Finds the longest TC_PUNCT spelling matching at source+i.
/// @return The matched token kind, or TK_INVALID if none matches.
/// @param out_len set to the length of the match (undefined if TK_INVALID)
static enum TokenKind match_punct(
    const char* source, const size_t* i, size_t* out_len
);

struct TokenList lex(struct Arena* a, const char* source)
{
    struct TokenList tkl;
    DARRAY_INIT(a, &tkl, TOKENLIST_INIT_CAPACITY);

    size_t i = 0;
    char c = source[i];
    struct SourceLoc loc = { .line = 1, .col = 1 };

    while (c != '\0') {
        c = source[i];

        if (isdigit((unsigned char)c)) {
            DARRAY_ADD(a, &tkl, lex_literal(a, source, &i, &loc));
        }
        else if (isalpha((unsigned char)c) || c == '_') {
            DARRAY_ADD(a, &tkl, lex_ident_kw(a, source, &i, &loc));
        }
        else if (isspace((unsigned char)c)) {
            loc_step(source, &i, &loc);
        }
        else {
            size_t out_len = 0;
            enum TokenKind punct_match = match_punct(source, &i, &out_len);

            if (punct_match != TK_INVALID) {
                DARRAY_ADD(
                    a, &tkl, ((struct Token){
                        .kind = punct_match,
                        .loc = loc,
                        .value = NULL,
                    })
                );
                for (size_t j = 0; j < out_len; j++) {
                    loc_step(source, &i, &loc);
                }
            }
            else {
                DARRAY_ADD(
                    a, &tkl, ((struct Token){
                        .kind = TK_INVALID,
                        .loc = loc,
                        .value = NULL,
                    })
                );
                loc_step(source, &i, &loc);
            }
        }
        c = source[i];
    }

    DARRAY_ADD(
        a, &tkl, ((struct Token){ .kind = TK_END, .loc = loc, .value = NULL })
    );
    return tkl;
}

static char loc_step(const char* source, size_t* i, struct SourceLoc* loc)
{
    char c = source[*i];
    if (c == '\0') {
        return c;
    }

    (*i)++;
    if (c == '\n') {
        loc->line++;
        loc->col = 1;
    }
    else {
        loc->col++;
    }

    return c;
}

static struct Token lex_literal(
    struct Arena* a, const char* source, size_t* i, struct SourceLoc* loc
) {
    assert(isdigit((unsigned char)source[*i]));

    struct SourceLoc start = *loc;
    const char* start_ptr = source + *i;
    size_t len = 0;

    while (isdigit((unsigned char)source[*i])) {
        loc_step(source, i, loc);
        len++;
    }

    char* value = ARENA_CALLOC_LIST(a, len + 1, char);
    strncpy(value, start_ptr, len);
    value[len] = '\0';

    return (struct Token){ .kind = TK_LITERAL, .value = value, .loc = start };
}

static struct Token lex_ident_kw(
    struct Arena* a, const char* source, size_t* i, struct SourceLoc* loc
) {
    assert(isalpha((unsigned char)source[*i]) || source[*i] == '_');

    struct SourceLoc start = *loc;
    const char* start_ptr = source + *i;
    size_t len = 0;

    while (isalnum((unsigned char)source[*i]) || source[*i] == '_') {
        loc_step(source, i, loc);
        len++;
    }

    char* value = ARENA_CALLOC_LIST(a, len + 1, char);
    strncpy(value, start_ptr, len);
    value[len] = '\0';

    enum TokenKind kw_kind = token_kind_from_str(value);
    if (token_kind_is_kw(kw_kind)) {
        return (struct Token){ .kind = kw_kind, .value = NULL, .loc = start };
    }

    return (struct Token){ .kind = TK_IDENT, .value = value, .loc = start };
}

static enum TokenKind match_punct(
    const char* source, const size_t* i, size_t* out_len
) {
    enum TokenKind match = TK_INVALID;
    size_t longest_len = 0;

#define TOKEN(kind, spelling, class) \
    if ((class) == TC_PUNCT) { \
        size_t len = strlen(spelling); \
        if (strncmp(source + *i, spelling, len) == 0 && len > longest_len) { \
            longest_len = len; \
            match = kind; \
        } \
    }
#include "lexer/tokens.def"
#undef TOKEN

    *out_len = longest_len;
    return match;
}

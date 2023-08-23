#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "error.h"
#include "loc.h"
#include "tokens.h"
#include "ast.h"


/* Finds the given `keyword` in the in `tokens.h` defined array `keywords`,
 * and gives its index in this array back.
 * 
 * Args:
 * - `keyword`, the string you want to compare to the keywords array
 *
 * Returns
 * - The index of the keyword, or -1 if `keyword` is not in `keywords` 
*/
int32_t
find_keyword(const char *keyword);

/* Searches for the punctuation in the in `tokens.h` defined `punctuations`
 * array, writes the index to `punct_i`.
 * When no punctuation matches at the current `i`, write -1 to `punct_i`.
 * 
 * Args:
 * - `source`, the entire source code 
 * - `i`, the index of the current character
 * - `punct_len`, the length of the punctuation token,
 *		when calling this function externally, pass NULL
 * - `punct_i`, will be index of punct in `punctuations` (-1 if no punct)
 *
 * Returns
 * - `ERROR_MEMORY_ALLOCATION`, on memory allocation fail
*/
int32_t 
match_punct(struct Loc *loc, uint32_t *punct_len);

/* Tokenizes the next token of the given source code at the ith character.
 * The `source[*i]` char should be the first character of the desired token.
 * Writes the desired token to `struct Token *token`.
 * When this function returns, `source[*i]` will be one character 
 * after the token.
 *
 * Args
 * - `source`, the source code
 * - `i`, the index of the character (may get modified)
 * - `token`, token will be written to
 *
 * Returns 
 * - `ERROR_EOF_REACHED`, if `source[*i] == '\0'` / if you're at the eof
 * - `ERROR_TOKEN_INVALID`, if the token does not exist
 * - `ERROR_MEMORY_ALLOCATION`, on memory allocation fail
*/
struct Token
lex_next(struct Loc *loc);

/* Converts an ast from the given source code.
 *
 * Args
 * - `source`, the source code
 * - `root`, pointer to root node to write to
 *
 * Returns
 * - `ERROR_EOF_REACHED`, if `source[*i] == '\0'` / if you're at the eof
 * - `ERROR_TOKEN_INVALID`, if a token does not exist
 * - `ERROR_MEMORY_ALLOCATION`, on memory allocation fail
*/
struct Node
parse(const char *source);

/* Parses a statement beginning with a keyword.
 *
 * Args:
 * - `source`, the source code
 * - `i`, current index in source code
 * - `token`, the keyword token
 * - `parent`, the parent node
*/
void
parse_keyword(struct Loc *loc, struct Token token, struct Node *parent);

#endif /* PARSER_H */

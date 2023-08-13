#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "error.h"
#include "tokens.h"


bool
is_digit(char c);

bool
is_ident_start(char c);

bool
is_ident_char(char c);

/* Finds the given `keyword` in the in `tokens.h` defined array `keywords`,
 * and gives the index in this array back.
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
 * array, returns the index. When no punctuation matches at the current `i`,
 * punct_len will equal 0.
 * 
 * Args:
 * - `source`, the entire source code 
 * - `i`, the index of the current character
 * - `punct_len`, the length of the punctuation token,
 *		when calling this function externally, pass NULL
 *
 * Returns
 * - The index of the punctuation, or -1 if `punct` is not in `punctuations` 
*/
int32_t 
match_punct(const char *source, int32_t i, uint32_t *punct_len);

/* Tokenizes the next token of the given source code at the ith character.
 * The `source[*i]` char should be the first character of the desired token.
 * Writes the desired token to `struct Token *token`.
 * When this function returns, `source[*i]` will be one character 
 * after the token.
 *
 * Args
 * - `source`, the source code
 * - `i`, the index of the character (may get modified)
 * - `token`, token will be written to or NULL if no token
 *
 * Returns 
 * - `ERROR_EOF_REACHED`, if `source[*i] == '\0'` / if you're at the eof
 * - `ERROR_TOKEN_INVALID`, if the token does not exist
*/
struct Error
lex_next(const char *source, int32_t *i, struct Token *token);

struct Error
parse(const char *source);

#endif /* PARSER_H */
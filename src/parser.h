#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include "error.h"
#include "tokens.h"


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
struct Error 
match_punct(
	const char *source,
	int32_t i,
	uint32_t *punct_len,
	int32_t *punct_i);

/* Tokenizes the next token of the given source code at the ith character.
 * The `source[*i]` char should be the first character of the desired token.
 * Writes the desired token to `struct Token *token`.
 * When this function returns, `source[*i]` will be one character 
 * after the token. Skips whitespace chars at the beginning.
 *
 * Args
 * - `source`, the source code
 * - `i`, the index of the character (may get modified)
 * - `token`, token will be written to or NULL if no token
 *
 * Returns 
 * - `ERROR_EOF_REACHED`, if `source[*i] == '\0'` / if you're at the eof
 * - `ERROR_TOKEN_INVALID`, if the token does not exist
 * - `ERROR_MEMORY_ALLOCATION`, on memory allocation fail
*/
struct Error
lex_next(const char *source, int32_t *i, struct Token *token);

struct Error
parse(const char *source);

#endif /* PARSER_H */

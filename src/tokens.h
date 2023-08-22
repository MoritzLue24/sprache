#ifndef TOKENS_H
#define TOKENS_H


enum TokenType 
{
	TT_KEYWORD,
	TT_IDENTIFIER,
	TT_PUNCT,
	TT_LITERAL,
};

extern const char *const keywords[3];

extern const char *const punctuations[6];

struct Token
{
	enum TokenType type;
	const char* value;
};

/* Frees the value field of a token, if it has been allocated manually.
 * (some of the token types have manually allocated values, some not)
 *
 * Args:
 * - `token`, the token to free
*/
void
free_token(struct Token token);

#endif /* TOKENS_H */

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

#endif /* TOKENS_H */

#ifndef TOKENS_H
#define TOKENS_H


enum TokenType 
{
	TT_KEYWORD,
	TT_IDENTIFIER,
	TT_PUNCT,
	TT_LITERAL,
};

const char *const keywords[] = {
	"fn",
	"return",
	"int32",
};

const char *const punctuations[] = {
	"(",
	")",
	"{",
	"}",
	";",
	"->",
};

struct Token
{
	enum TokenType type;
	const char* value;
};

#endif /* TOKENS_H */

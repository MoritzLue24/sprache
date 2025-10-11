#ifndef TOKENS_H
#define TOKENS_H

#include "loc.h"


enum TokenType 
{
	TT_EOF,
	TT_KEYWORD,
	TT_IDENTIFIER,
	TT_PUNCT,
	TT_LITERAL
};

enum KeywordType
{
	KW_INVALID,		/* first, so Token.KeywordType is invalid by default */

	KW_FUNCTION,
	KW_RETURN
};

enum PunctuationType
{
	PUNCT_INVALID,	/* first, so Token.PunctuationType is invalid by default */

	PUNCT_PAREN_OPEN,
	PUNCT_PAREN_CLOSE,
	PUNCT_BRACE_OPEN,
	PUNCT_BRACE_CLOSE
};

extern const char *const keywords[3];

extern const char *const punctuations[5];

struct Token
{
	enum TokenType type;
	struct Loc start;
	struct Loc end;
	char *value;
	union {
		enum KeywordType kw_type;
		enum PunctuationType punct_type;
	};
};

#endif /* TOKENS_H */

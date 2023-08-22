#include "tokens.h"
#include <stdlib.h>


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

void
free_token(struct Token token)
{
	switch (token.type)
	{
        case TT_IDENTIFIER:
        case TT_LITERAL:
			free((void*)token.value);
			break;

		default:
		  return;
	}
}

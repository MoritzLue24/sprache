#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "utils.h"


int32_t
find_keyword(const char *keyword)
{
	for (uint32_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
		if (!strcmp(keywords[i], keyword))
			return i;
	return -1;
}

int32_t 
match_punct(struct Loc *loc, uint32_t *punct_len)
{
	uint32_t punctuations_len = sizeof(punctuations) / sizeof(punctuations[0]);

	uint32_t punct_longest = 0;
	if (punct_len == NULL)
	{
		/* Get the length of the punctuation with the longest characters */
		uint32_t current_len;
		for (uint32_t i = 0; i < punctuations_len; ++i)
			if ((current_len = strlen(punctuations[i])) > punct_longest)
				punct_longest = current_len;
		punct_len = &punct_longest;
	}

	for (uint32_t punct_i = 0; punct_i < punctuations_len; ++punct_i)
	{
		if (strlen(punctuations[punct_i]) != *punct_len)
			continue;

		/* Copy the substring source[i] to source[i + punct_len] */
		char *substr = malloc(*punct_len + 1);
		strncpy(substr, loc->source + loc->i, *punct_len);
		substr[*punct_len] = '\0';

		/* If the substr is the current punct_i string, return it */
		if (!strcmp(punctuations[punct_i], substr))
		{
			free(substr);
			return punct_i;
		}
		free(substr);
	}

	--(*punct_len);
	if (*punct_len > 0)
		return match_punct(loc, punct_len);
	return -1;
}

struct Token
lex_next(struct Loc *loc)
{
	if (loc->c == '\0')
		fail_spr(ERROR_EOF_REACHED, *loc, NULL);

	if (is_digit(loc->source[loc->i]))
	{
		/* Loop over all digit chars in source code 
		(the index `i` will be on the last valid digit char) */
		struct Loc start_loc = *loc;
		while (is_digit(loc->source[loc->i + 1]))
			step(loc);

		char* value = malloc(loc->i - start_loc.i + 2);
		if (value == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(value, loc->source + start_loc.i, loc->i - start_loc.i + 1);
		value[loc->i - start_loc.i + 1] = '\0';

		/* currently, `i` is the last valid digit char so add one */
		step(loc);
		return (struct Token){ TT_LITERAL, value };
	}

	if (is_ident_start(loc->c))
	{
		/* Loop over all valid identifier chars in source code
		(the index `i` will be on the last valid ident char) */
		struct Loc start_loc = *loc;
		step(loc);
		
		while (is_ident_char(loc->source[loc->i + 1]))
			step(loc);

		char* ident = malloc(loc->i - start_loc.i + 2);
		if (ident == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		/* copy content from start_i to current i to `ident` */
		strncpy(ident, loc->source + start_loc.i, loc->i - start_loc.i + 1);
		ident[loc->i - start_loc.i + 1] = '\0';

		/* currently, `i` is the last valid ident char so add one */
		step(loc);

		int32_t kw_i;
		if ((kw_i = find_keyword(ident)) != -1)
			return (struct Token){ TT_KEYWORD, keywords[kw_i] };
		else
			return (struct Token){ TT_IDENTIFIER, ident };
	}

	int32_t punct_i = match_punct(loc, NULL);
	if (punct_i != -1)
	{
		for (uint64_t _ = 0; _ < strlen(punctuations[punct_i]); ++_)
			step(loc);

		return (struct Token){ TT_PUNCT, punctuations[punct_i] };
	}
	
	fail_spr(ERROR_TOKEN_INVALID, *loc, NULL);
	return (struct Token){ TT_INVALID, NULL };
}

struct Node
parse(const char *source)
{
	struct Loc loc = { source, source[0], 0, 1, 1 };
	while (loc.c != '\0')
	{
		struct Token token = lex_next(&loc);
		printf("%i, %s\n", token.type, token.value);
		/* switch (token.type) */
		/* { */
		/* 	case TT_KEYWORD: */
		/* 		parse_keyword(source, &i, token, root); */
		/* 		break; */
		/* 	case TT_IDENTIFIER: */
		/* 	case TT_PUNCT: */
		/* 	case TT_LITERAL: */
		/* 		break; */
		/* } */
		while (
			loc.c == ' ' ||
			loc.c == '\t' ||
			loc.c == '\n' ||
			loc.c == '\r'
		) {
			step(&loc);
		}
		free_token(token);
	}
}

/* struct Error */ 
/* parse_keyword( */
/* 	const char *source, */
/* 	int32_t *i, */
/* 	struct Token token, */
/* 	struct Node *parent) */
/* { */
/* 	const char *kw = token.value; */
/* 	if (!strcmp(kw, "fn")) */
/* 	{ */
/* 		struct Token name_token; */
/* 		struct Error err = lex_next(source, i, &name_token); */
/* 		if (err.code != ERROR_NONE) */
/* 			return err; */

/* 		if (name_token.type != TT_IDENTIFIER) */
/* 			return (struct Error){ ERROR_SYNTAX_INVALID, "identifier expected" }; */

		
/* 	} */
/* 	else if (!strcmp(kw, "return")) */
/* 	{ */

/* 	} */
/* 	else { */
/* 		// return (struct Error){ ERROR_INVALID_SYNTAX, NULL }; */
/* 	} */
	
/* 	return OK; */
/* } */

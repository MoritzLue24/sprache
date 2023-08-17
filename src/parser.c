#include "parser.h"
#include <stdlib.h>
#include <string.h>
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
match_punct(const char *source, int32_t i, uint32_t *punct_len)
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
		strncpy(substr, source + i, *punct_len);
		substr[*punct_len] = '\0';

		/* If the substr is the current punct_i string, return it */
		if (!strcmp(punctuations[punct_i], substr))
			return punct_i;
	}

	--(*punct_len);
	if (*punct_len > 0)
		return match_punct(source, i, punct_len);
	return -1;
}

struct Error
lex_next(const char *source, int32_t *i, struct Token *token)
{
	skip_whitespaces(source, i);
	if (source[*i] == '\0')
		return (struct Error){ ERROR_EOF_REACHED, NULL };
	
	if (is_digit(source[*i]))
	{
		/* Loop over all digit chars in source code 
		(the index `i` will be on the last valid digit char) */
		int32_t start_i = *i;
		while (is_digit(source[*i + 1]))
			++(*i);

		char* value = malloc(*i - start_i + 2);
		if (value == NULL)
			return (struct Error){ ERROR_MEMORY_ALLOCATION, "malloc failed" };

		strncpy(value, source + start_i, *i - start_i + 1);
		value[*i - start_i + 1] = '\0';

		/* currently, `i` is the last valid digit char so add one */
		++(*i);

		token->type = TT_LITERAL;
		token->value = value;
		return (struct Error){ ERROR_NONE, NULL };
	}

	/* used for `match_punct` later in this function */
	int32_t punct_i;

	if (is_ident_start(source[*i]))
	{
		/* Loop over all valid identifier chars in source code
		(the index `i` will be on the last valid ident char) */
		int32_t start_i = (*i)++;
		
		while (is_ident_char(source[*i + 1]))
			++(*i);

		char* ident = malloc(*i - start_i + 2);
		if (ident == NULL)
			return (struct Error){ ERROR_MEMORY_ALLOCATION, "malloc failed" };

		/* copy content from start_i to current i to `ident` */
		strncpy(ident, source + start_i, *i - start_i + 1);
		ident[*i - start_i + 1] = '\0';

		/* currently, `i` is the last valid ident char so add one */
		++(*i);

		int32_t kw_i;
		if ((kw_i = find_keyword(ident)) != -1)
		{
			token->type = TT_KEYWORD;
			token->value = keywords[kw_i];
		}
		else
		{
			token->type = TT_IDENTIFIER;
			token->value = ident;
		}
		return (struct Error){ ERROR_NONE, NULL };
	}

	else if ((punct_i = match_punct(source, *i, NULL)) != -1)
	{
		*i += strlen(punctuations[punct_i]);
		token->type = TT_PUNCT;
		token->value = punctuations[punct_i];
		return (struct Error){ ERROR_NONE, NULL };
	}
	
	token = NULL;
	return (struct Error){ ERROR_TOKEN_INVALID, NULL };
}

struct Error
parse(const char *source)
{
	int32_t i = 0;
	while (source[i] != '\0')
	{
		/* TODO */
		++i;	
	}
	return (struct Error){ ERROR_NONE, NULL };
}

#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "error.h"
#include "utils.h"


enum KeywordType
find_keyword(const char *keyword)
{
	for (uint8_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
		if (!strcmp(keywords[i], keyword))
			return i;
	return KW_INVALID;
}

enum PunctuationType 
match_punct(struct Loc *loc, size_t *punct_len)
{
	size_t list_len = sizeof(punctuations) / sizeof(punctuations[0]);
	size_t longest_len = 0;

	if (punct_len == NULL)
	{
		/* Get the length of the punctuation with the longest characters */
		size_t current_len;
		for (size_t i = 0; i < list_len; ++i) {
			if ((current_len = strlen(punctuations[i])) > longest_len)
				longest_len = current_len;
		}
		punct_len = &longest_len;
	}

	for (size_t i = 0; i < list_len; ++i)
	{
		if (strlen(punctuations[i]) != *punct_len)
			continue;

		char *value = malloc(*punct_len + 1);
		if (value == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(value, loc->source + loc->i, *punct_len);
		value[*punct_len] = '\0';

		if (!strcmp(punctuations[i], value))
		{
			free(value);
			return i;
		}
		free(value);
	}

	--(*punct_len);
	if (*punct_len > 0)
		return match_punct(loc, punct_len);
	return PUNCT_INVALID;
}

struct Token
lex_next(struct Loc *loc)
{
	skip_whitespace(loc);
	if (loc->c == '\0')
		fail_spr(ERROR_EOF_REACHED, *loc, NULL);

	struct Token token = {};

	if (isdigit(loc->source[loc->i]))
	{
		token.type = TT_LITERAL;
		token.start = *loc;
		while (isdigit(loc->source[loc->i + 1]))
			step(loc);

		char* value = malloc(loc->i - token.start.i + 2);
		if (value == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(value, loc->source + token.start.i, loc->i - token.start.i + 1);
		value[loc->i - token.start.i + 1] = '\0';
		token.value = value;

		/* currently, `i` is the last valid digit char so add one */
		token.end = *loc;
		step(loc);
		return token;
	}

	if (isalpha(loc->c) || loc->c == '_')
	{
		token.start = *loc;
		while (
			isalpha(loc->source[loc->i + 1]) || 
			isdigit(loc->source[loc->i + 1]) || 
			loc->source[loc->i + 1] == '_'
		)
			step(loc);

		char* value = malloc(loc->i - token.start.i + 2);	/*  */
		if (value == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(value, loc->source + token.start.i, loc->i - token.start.i + 1);
		value[loc->i - token.start.i + 1] = '\0';

		/* currently, `i` is the last valid ident char so add one */
		token.end = *loc;
		step(loc);

		enum KeywordType kw_type = find_keyword(value);
		if (kw_type != KW_INVALID)
		{
			token.type = TT_KEYWORD;
			token.kw_type = kw_type;
			token.value = NULL;
			free(value);
		}
		else
		{
			token.type = TT_IDENTIFIER;
			token.value = value;
		}
		return token;
	}

	enum PunctuationType punct_type = match_punct(loc, NULL);
	if (punct_type != PUNCT_INVALID)
	{
		token.type = TT_PUNCT;
		token.start = *loc;
		token.punct_type = punct_type;

		uint8_t punct_len = strlen(punctuations[punct_type]);

		for (uint8_t i = 0; i < punct_len; i++)
		{
			if (i == punct_len - 1)
				token.end = *loc;
			step(loc);
		}

		token.value = NULL;
		return token;
	}

	fail_spr(ERROR_TOKEN_INVALID, *loc, NULL);
	return (struct Token){};
}

struct Node
parse(const char *source)
{
	struct Loc loc = { source, source[0], 0, 1, 1 };
    return parse_block(&loc, lex_next(&loc));
}

struct Node
parse_block(struct Loc *loc, struct Token token)
{
	if (token.type != TT_PUNCT && token.punct_type != PUNCT_BRACE_OPEN)
		fail_spr(ERROR_SYNTAX_INVALID, token.start, "'{' expected");

	struct Node block = {
		.kind = NODE_BLOCK,
		.start = token.start,
		.n_block = {
			.body = NULL,
			.count = 0
		}
	};

	struct Token current_t = lex_next(loc);
	while (current_t.type != TT_PUNCT && current_t.punct_type != PUNCT_BRACE_CLOSE)
	{	
		struct Node statement = parse_statement(loc, current_t);
		
		block.n_block.count++;
		if (block.n_block.body == NULL)
			block.n_block.body = malloc(sizeof(struct Node));
		else
			block.n_block.body = realloc(block.n_block.body, sizeof(struct Node) * block.n_block.count);
		if (block.n_block.body == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");
		block.n_block.body[block.n_block.count - 1] = statement;

		current_t = lex_next(loc);
	}
	block.end = current_t.end;
	return block;
}

struct Node
parse_statement(struct Loc *loc, struct Token token)
{
	switch (token.type)
	{
		case TT_KEYWORD:
			switch (token.kw_type)
			{
				case KW_FUNCTION:
					return parse_function(loc, token);
					break;
                case KW_RETURN:
                    return parse_return(loc, token);
				default:
					fail_spr(ERROR_SYNTAX_INVALID, *loc, "keyword not implemented");
			}
			break;
		default:
			fail_spr(ERROR_SYNTAX_INVALID, *loc, "unexpected token");
	}
	return (struct Node){};
}

struct Node
parse_function(struct Loc *loc, struct Token token)
{
	struct Node function = {
		.kind = NODE_FUNCTION,
		.start = token.start,
		.n_function = {}
	};

	struct Token name = lex_next(loc);
	if (name.type != TT_IDENTIFIER)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "identifier expected");
	function.n_function.name_token = name;
	
	struct Token open_paren = lex_next(loc);
	if (open_paren.type != TT_PUNCT && open_paren.punct_type != PUNCT_PAREN_OPEN)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "'(' expected");

	struct Token close_paren = lex_next(loc);
	if (close_paren.type != TT_PUNCT && close_paren.punct_type != PUNCT_PAREN_CLOSE)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "')' expected");

	function.n_function.block_node = malloc(sizeof(struct Node));
	if (function.n_function.block_node == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");
	*function.n_function.block_node = parse_block(loc, lex_next(loc));

	function.end = function.n_function.block_node->end;
	return function;
}

struct Node
parse_return(struct Loc *loc, struct Token token)
{
	struct Node return_node = {
		.kind = NODE_RETURN,
		.start = token.start,
		.n_return = {
			.value_node = NULL
		}
	};
	
    struct Token value = lex_next(loc);
    if (value.type != TT_LITERAL)
	{
    	fail_spr(ERROR_SYNTAX_INVALID, *loc, "literal expected");
	}

	return_node.n_return.value_node = malloc(sizeof(struct Node));
	if (return_node.n_return.value_node == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

	*return_node.n_return.value_node = (struct Node) {
		.kind = NODE_LITERAL,
		.start = value.start,
		.end = value.end,
		.n_literal = { .value_token = value }
	};

	return_node.end = value.end;
    return return_node;
}

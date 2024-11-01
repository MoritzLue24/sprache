#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "utils.h"


enum KeywordType
find_keyword(const char *keyword)
{
	for (uint32_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
		if (!strcmp(keywords[i], keyword))
			return i + 1;	/* Add one because the first type is KW_INVALID */
	return KW_INVALID;
}

enum PunctuationType 
match_punct(struct Loc *loc, uint32_t *punct_len)
{
	uint32_t punctuations_len = sizeof(punctuations) / sizeof(punctuations[0]);

	uint32_t punct_longest = 0;
	if (punct_len == NULL)
	{
		/* Get the length of the punctuation with the longest characters */
		uint32_t current_len;
		for (uint32_t i = 0; i < punctuations_len; ++i) {
			if ((current_len = strlen(punctuations[i])) > punct_longest)
				punct_longest = current_len;
		}
		punct_len = &punct_longest;
	}

	for (uint32_t punct_i = 0; punct_i < punctuations_len; ++punct_i)
	{
		if (strlen(punctuations[punct_i]) != *punct_len)
			continue;

		/* Copy the substring source[i] to source[i + punct_len] */
		char *substr = malloc(*punct_len + 1);
		if (substr == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(substr, loc->source + loc->i, *punct_len);
		substr[*punct_len] = '\0';

		/* If the substr is the current punct_i string, return it */
		if (!strcmp(punctuations[punct_i], substr))
		{
			free(substr);
			return punct_i + 1;		/* Add one because the first type is KW_INVALID */
		}
		free(substr);
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

	struct Token token;

	if (is_digit(loc->source[loc->i]))
	{
		token.type = TT_LITERAL;

		/* Loop over all digit chars in source code 
		(the index `i` will be on the last valid digit char) */
		token.start = *loc;
		while (is_digit(loc->source[loc->i + 1]))
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

	if (is_ident_start(loc->c))
	{
		/* Loop over all valid identifier chars in source code
		(the index `i` will be on the last valid ident char) */
		token.start = *loc;
		step(loc);
		
		while (is_ident_char(loc->source[loc->i + 1]))
			step(loc);

		char* ident = malloc(loc->i - token.start.i + 2);
		if (ident == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		/* copy content from start_i to current i to `ident` */
		strncpy(ident, loc->source + token.start.i, loc->i - token.start.i + 1);
		ident[loc->i - token.start.i + 1] = '\0';

		/* currently, `i` is the last valid ident char so add one */
		token.end = *loc;
		step(loc);

		enum KeywordType kw_type = find_keyword(ident);
		if (kw_type != KW_INVALID)
		{
			token.type = TT_KEYWORD;
			token.kw_type = kw_type;
			token.value = NULL;
		}
		else
		{
			token.type = TT_IDENTIFIER;
			token.value = ident;
		}
		return token;
	}

	enum PunctuationType punct_type = match_punct(loc, NULL);
	if (punct_type != PUNCT_INVALID)
	{
		token.type = TT_PUNCT;
		token.start = *loc;
		token.punct_type = punct_type;

		/* Subtract one because the punct type != the index, see mtach_punct*/
		uint8_t punct_len = strlen(punctuations[punct_type - 1]);

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

	struct Node root = { NODE_ROOT, .n_root = { malloc(sizeof(struct NodeListElement)) } };
	if (root.n_root.body == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

	root.n_root.body->self = NULL;
	root.n_root.body->next = NULL;

	while (loc.c != '\0')
	{
		struct Token token = lex_next(&loc);
		struct Node statement = parse_statement(&loc, token);

        if (root.n_root.body->self == NULL)
            insert_node_list(root.n_root.body, statement, 0);
        else 
            append_node_list(root.n_root.body, statement);
        skip_whitespace(&loc);
	}
	return root;
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
					return parse_function(loc);
					break;
                case KW_RETURN:
                    return parse_return(loc);
				default:
					fail_spr(ERROR_SYNTAX_INVALID, *loc, "keyword not implemented");
			}
			break;
		default:
			fail_spr(ERROR_SYNTAX_INVALID, *loc, "token not implemented");
	}
	return (struct Node){};
}

struct Node
parse_function(struct Loc *loc)
{
	struct Token name = lex_next(loc);
	if (name.type != TT_IDENTIFIER)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "identifier expected");
	
	struct Token open_paren = lex_next(loc);
	if (open_paren.type != TT_PUNCT && open_paren.punct_type != PUNCT_PAREN_OPEN)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "'(' expected");

	/* TODO: arguments */

	struct Token close_paren = lex_next(loc);
	if (close_paren.type != TT_PUNCT && close_paren.punct_type != PUNCT_PAREN_CLOSE)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "')' expected");

	struct Token open_brace = lex_next(loc);
	if (open_brace.type != TT_PUNCT && open_brace.punct_type != PUNCT_BRACE_OPEN)
		fail_spr(ERROR_SYNTAX_INVALID, *loc, "'{' expected");

	struct NodeListElement* root_element = malloc(sizeof(struct NodeListElement));
	if (root_element == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");
	root_element->self = NULL;
	root_element->next = NULL;

	struct Token current_t = lex_next(loc);
    if (current_t.type != TT_PUNCT && current_t.punct_type != PUNCT_BRACE_CLOSE)
        insert_node_list(root_element, parse_statement(loc, current_t), 0);
    else
        return (struct Node) {
            NODE_FUNCTION,
                .n_function = (struct Node_Function) {
                    name, .args = NULL, .body = root_element 
                }
        };

    while (1)
	{
        skip_whitespace(loc);
        if (loc->c == '\0')
            fail(ERROR_EOF_REACHED, "expected '}'");

		current_t = lex_next(loc);
        skip_whitespace(loc);

        if (current_t.type == TT_PUNCT && current_t.punct_type == PUNCT_BRACE_CLOSE)
            break;

		struct Node statement = parse_statement(loc, current_t);
		append_node_list(root_element, statement);
	}

	return (struct Node) {
		NODE_FUNCTION,
		.n_function = (struct Node_Function) {
			name, .args = NULL, .body = root_element 
		}
	};
}

struct Node
parse_return(struct Loc *loc)
{
    struct Token value = lex_next(loc);
    if (value.type != TT_LITERAL)
        fail_spr(ERROR_SYNTAX_INVALID, *loc, "literal expected");

    struct Node *literal = malloc(sizeof(struct Node));
    if (literal == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    literal->kind = NODE_LITERAL;
    literal->n_literal = (struct Node_Literal) { .value = value };

    return (struct Node) {
        .kind = NODE_RETURN,
        .n_return = (struct Node_Return) {
            .value = literal        }
    };
}

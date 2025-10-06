#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "loc.h"
#include "tokens.h"
#include "parser.h"
#include "ast.h"
#include "gen.h"


void
test_all()
{
	test_tokens();
	test_ast();
}

void 
test_tokens()
{
	printf("Testing 'tokens'\n");

	const char *source = read_file("examples/debug.s");

	struct Loc loc = { source, source[0], 0, 1, 1 };
	while (loc.c != '\0')
	{
		struct Token token = lex_next(&loc);
		printf(
			"TT %i, KW_T %i, PUNCT_T %i ; %s\n",
			token.type, 
			token.kw_type,
			token.punct_type,
			token.value
		);

		while (
			loc.c == ' ' ||
			loc.c == '\t' ||
			loc.c == '\n' ||
			loc.c == '\r'
		) {
			step(&loc);
		}
	}
}

void 
test_ast()
{
	printf("Testing 'ast'\n");

	const char *source = read_file("examples/debug.s");
    struct Node root = parse(source);

	print_node(root, 0);
    free_node(root);
	free((void*)source);
}

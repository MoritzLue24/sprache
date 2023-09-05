#ifdef TEST
#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "loc.h"
#include "tokens.h"
#include "parser.h"
#include "ast.h"


void
test_all()
{
	test_tokens();
	test_node_list();
	test_ast();
}

void 
test_tokens()
{
	printf("Testing 'tokens'\n");

	const char *source = read_file("examples/latest.s");

	struct Loc loc = { source, source[0], 0, 1, 1 };
	while (loc.c != '\0')
	{
		struct Token token = lex_next(&loc);
		printf("%i, %.*s\n", token.type, 
			token.end.i - token.start.i + 1, 
			token.start.source + token.start.i);

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
test_node_list()
{
	printf("Testing 'node_list'\n");

	struct NodeListElement *root = malloc(sizeof(struct Node));
	root->self = NULL;
	root->next = NULL;
	
	struct Node some_node = {
	 	.kind = NODE_LITERAL,
	 	.n_literal = (struct Node_Literal) { (struct Token) { .type = TT_LITERAL }},
	};

    insert_node_list(root, some_node, 0);
    append_node_list(root, some_node);

    printf("%p\n", root->next->self);

	print_node_list(root, 0);
	free_node_list(root);
}

void 
test_ast()
{
	printf("Testing 'ast'\n");
	
	const char *source = read_file("examples/latest.s");
    struct Node root = parse(source);
    
	print_node(root, 0);
    free_node(root);
	free((void*)source);
}

#endif

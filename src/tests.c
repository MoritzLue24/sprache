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
#include "mem_stream.h"
#include "gen.h"


void
test_all()
{
	test_tokens();
	test_node_list();
	test_ast();
	test_gen();
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
test_node_list()
{
	/* TODO: is currently crashing -> fix it */
	printf("Testing 'node_list'\n");

	struct NodeListElement *root = malloc(sizeof(struct Node));
	if (root == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

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

	const char *source = read_file("examples/debug.s");
    struct Node root = parse(source);

	print_node(root, 0);
    free_node(root);
	free((void*)source);
}

void
test_mem_stream()
{
	struct MemStream *stream = stream_open(0);
	stream_write(stream, "%i", 40 + 2);
	stream_write(stream, "%i", 30 + 2);
	printf("%s, size: %i", stream->buf, stream->size);
	free_stream(stream);
}

void
test_gen()
{
	printf("Testing 'gen'\n");

	const char *source = read_file("examples/debug.s");
	struct Node root = parse(source);

	struct MemStream *asm_stream = gen(root);

	free_node(root);
	free((void*)source);
	free_stream(asm_stream);
}

#endif

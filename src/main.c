#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "tests.h"
#include "error.h"
#include "utils.h"
#include "parser.h"
#include "ast.h"
#include "gen.h"
#include "mem_stream.h"


void
print_help()
{
	printf(
		"sprache <command> [<args>]\n\n"
		"command            summary\n"
		"-------            -------\n"
		"help				Shows this message\n"
		"file <path>        Compiles a file\n"
		"buffer <source>    Compiles given source code\n"
	);
}

int32_t
main(int argc, char **argv)
{
#ifdef TEST
	if (argc == 0)
	{
		test_all();
		return 0;
	}
	for (int32_t i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "tokens"))
			test_tokens();
		else if (!strcmp(argv[i], "node_list"))
			test_node_list();
		else if (!strcmp(argv[i], "ast"))
			test_ast();
		else if (!strcmp(argv[i], "mem_stream"))
			test_mem_stream();
		else if (!strcmp(argv[i], "gen"))
			test_gen();
	}
	return 0;

#else
	if (argc == 1)
	{
		fprintf(stderr, "command expected");
		return -1;
	}
	if (!strcmp(argv[1], "help"))
	{
		print_help();
		return 0;
	}
	else if (!strcmp(argv[1], "file"))
	{
		const char *source = read_file(argv[2]);

		struct Node root = parse(source);
		printf("%s\n", root.n_root.body->self->n_function.body->self->n_return.value->n_literal.value.value);
		print_node(root, 0);
		//struct MemStream *stream = gen(root);
		//printf("%s", stream->buf);

		//free_stream(stream);
		free_node(root);
		free((void*)source);
		return 0;
	}
	else if (!strcmp(argv[1], "buffer"))
	{
		/* const char* source = argv[2]; */
		/* TODO: pass source into lexer (or tokenizer) */
		return 0;
	}
	fprintf(stderr, "invalid command");
	return -1;
#endif
}

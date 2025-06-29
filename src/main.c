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
		"Usage: sprache <file> [options]\n\n"
		"Options:\n"
		"\t-h, --help			Shows this message\n"
		"\t-o, --output			Specifies the executable output\n"
		/* TODO: "\t-s, --asm [<file>]		Generates an assembly file when compiling\n" */
	);
}

void
compile(char *in_file, char *asm_file)
{
	char *source = read_file(in_file);

	struct Node root = parse(source);
	printf("AST:\n");
	print_node(root, 0);

	/* TODO: with examples/return_421.s, there is a '1' at the end of stream */
	struct MemStream *stream = gen(root.n_root);
	printf("\nAssembled Code:\n%s\n", stream->buf);
	stream_write_to_file(stream, asm_file);

	free_stream(stream);
	free_node(root);
	free(source);
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
	char *in_file = NULL;
	char *asm_file = NULL;
	char *out_file = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
		{
			print_help();
			return 0;
		}
		else if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o"))
		{
			if (i + 1 < argc)
				out_file = argv[++i];
			else
				fail(ERROR_CLI_USAGE, "option '%s' requires an argument", argv[i]);
		}
		else if (strncmp(argv[i], "-", 1))
			in_file = argv[i];
		else
			fail(ERROR_CLI_USAGE, "invalid option '%s'", argv[i]);
	}

	if (in_file == NULL)
		fail(ERROR_CLI_USAGE, "positional argument 'file' is missing");
	else
	{
		char *ext = strrchr(in_file, '.');
		if (ext == NULL || strcmp(ext, ".s"))
			fail(ERROR_FILEPATH_INVALID, "'%s' extension '.s' is missing", in_file);

		size_t asm_file_len = strlen(in_file) - strlen(".s") + strlen(".asm") + 1;
		asm_file = malloc(asm_file_len);
		if (asm_file == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

		strncpy(asm_file, in_file, strlen(in_file) - strlen(".s"));
		strcat(asm_file, ".asm");
	}

	compile(in_file, asm_file);
	free(asm_file);

	return 0;
#endif
}

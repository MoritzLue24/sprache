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


void
print_help()
{
	printf(
		"Usage: sprache <file> [options]\n\n"
		"Options:\n"
		"\t-h, --help			Shows this message\n"
		"\t-a, --asm <file>		Generates an assembly file when compiling\n"
		//"\t-o, --output	<file>	Specifies the executable output\n"
	);
}

void
compile(char *in_file, char *asm_file)
{
	char *source = read_file(in_file);

	struct Node root = parse(source);
	printf("AST:\n");
	print_node(root, 0);

	char *asm_code = gen(root);
	printf("\nAssembled Code:\n%s\n", asm_code);
	/*
	FILE *asm_f = fopen(asm_file, "w");
	if (asm_f == NULL)
		fail(ERROR_FILEPATH_INVALID, "failed opening asm file '%s'", asm_file);
	
	fprintf(asm_f, "%s", stream);
	fclose(asm_f);
	*/
	free(asm_code);
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

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
		{
			print_help();
			return 0;
		}
		else if (!strcmp(argv[i], "--asm") || !strcmp(argv[i], "-a"))
		{
			if (i + 1 < argc)
				asm_file = argv[++i];
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
		
		if (asm_file == NULL)
		{
			size_t base_len = strlen(in_file) - strlen(".s");
			asm_file = malloc(base_len + strlen(".asm") + 1);
			if (asm_file == NULL)
				fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

			strncpy(asm_file, in_file, base_len);
			asm_file[base_len] = '\0';
			strcat(asm_file, ".asm");
		}
	}

	compile(in_file, asm_file);
	free(asm_file);

	return 0;
#endif
}

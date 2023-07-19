#include <stdio.h>
#include <string.h>
#include "stdlib.h"

void print_help() {
	printf(
		"sprache <command> [<args>]\n\n"
		"command            summary\n"
		"-------            -------\n"
		"help				Shows this message\n"
		"file <path>        Compiles a file\n"
		"buffer <source>    Compiles given source code\n"
	);
}

char* read_file(const char* path) {
	FILE* file = fopen(path, "rb");
	if (!file) {
		perror(NULL);
		exit(-1);
	}
	
	fseek(file, 0, SEEK_END);
    long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* source = malloc(size + 1);
	fread(source, sizeof(char), size, file);
	source[size] = '\0';

	fclose(file);
	return source;
}

int main(int argc, char** argv) {
	if (argc == 1) {
		fprintf(stderr, "command expected");
		return -1;
	}

	if (!strcmp(argv[1], "help")) {
		print_help();
		return 0;
	}
	else if (!strcmp(argv[1], "file")) {
		const char* source = read_file(argv[2]);
		// TODO: Pass source into lexer (or tokenizer)
		free((void*)source);
		return 0;
	}
	else if (!strcmp(argv[1], "buffer")) {
		const char* source = argv[2];
		// TODO: Pass source into lexer (or tokenizer)
		return 0;
	}
	fprintf(stderr, "invalid command");
	return -1;
}

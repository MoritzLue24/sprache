#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


char*
read_file(const char *path)
{
	FILE* file = fopen(path, "rb");
	if (!file) {
		perror(NULL);
		exit(-1);
	}
	
	fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* source = malloc(size + 1);
	fread(source, sizeof(char), size, file);
	source[size] = '\0';

	fclose(file);
	return source;
}

bool
is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

bool
is_ident_start(char c)
{
	return ((c >= 'A' && c <= 'Z') || 
			(c >= 'a' && c <= 'z') ||
			c == '_');
}

bool
is_ident_char(char c)
{
	return is_ident_start(c) || is_digit(c);
}

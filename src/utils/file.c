#include "utils/file.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils/xalloc.h"

char* read_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
        fprintf(stderr, "File not found: '%s'", path);
		return NULL;
    }

	fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* source = xmalloc(size + 1);
	fread(source, sizeof(char), size, file);
	source[size] = '\0';

	fclose(file);
	return source;
}

bool file_exists(const char* path)
{
    FILE *file;
    if ((file = fopen(path, "r")) != NULL) {
        fclose(file);
        return true;
    }
    return false;
}
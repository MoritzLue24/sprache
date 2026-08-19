#include <stdlib.h>
#include "utils/file.h"

FILE* openw_file(const char* filename)
{
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    return f;
}

const char* read_file(struct Arena* a, const char* path)
{
    FILE* file = fopen(path, "rb");
	if (file == NULL) {
        fprintf(stderr, "File not found: '%s'\n", path);
        exit(1);
    }

	fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

    char* source = ARENA_CALLOC_LIST(a, size + 1, char);
	fread(source, sizeof(char), size, file);
	source[size] = '\0';

	fclose(file);
	return source;
}
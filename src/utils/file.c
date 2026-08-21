#include "utils/file.h"
#include "utils/arena.h"
#include <stdlib.h>

FILE* file_openw(const char* filename)
{
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: File not found: '%s'\n", filename);
        exit(1);
    }
    return f;
}

const char* file_read(struct Arena* a, const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: File not found: '%s'\n", path);
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

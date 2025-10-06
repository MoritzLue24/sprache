#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "error.h"


char*
read_file(const char *path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		fail(ERROR_FILEPATH_INVALID, path);
	}
	
	fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* source = malloc(size + 1);
	if (source == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

	fread(source, sizeof(char), size, file);
	source[size] = '\0';

	fclose(file);
	return source;
}

char*
format(const char *source, ...)
{
    va_list args;
    va_start(args, source);
    int size = vsnprintf(NULL, 0, source, args);
    va_end(args);

    char *formatted = malloc(size + 1);
    if (formatted == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    va_start(args, source);
    vsnprintf(formatted, size + 1, source, args);
    va_end(args);

	return formatted;
}

void
skip_whitespace(struct Loc *loc)
{
    while (
        loc->c == ' ' ||
        loc->c == '\t' ||
        loc->c == '\n' ||
        loc->c == '\r'
    ) {
        step(loc);
    }
}

char *
append_string(char *dest, const char *source, ...)
{
	if (dest == NULL)
	{
		dest = malloc(1);
		if (dest == NULL)
			fail(ERROR_MEMORY_ALLOCATION, "malloc failed");
		dest[0] = '\0';
	}

	va_list args;
    va_start(args, source);
    int f_size = vsnprintf(NULL, 0, source, args);
    va_end(args);

    char *formatted = malloc(f_size + 1);
    if (formatted == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    va_start(args, source);
    vsnprintf(formatted, f_size + 1, source, args);
    va_end(args);

	size_t old_len = strlen(dest);
	size_t add_len = strlen(formatted);

	dest = realloc(dest, old_len + add_len + 1);
	if (dest == NULL)
		fail(ERROR_MEMORY_ALLOCATION, "realloc failed");

	strcat(dest, formatted);
	free(formatted);
	return dest;
}
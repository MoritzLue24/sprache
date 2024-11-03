#include "mem_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "error.h"


struct MemStream*
stream_open(size_t size)
{
    struct MemStream *stream = (struct MemStream*)malloc(sizeof(struct MemStream));
    if (stream == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    stream->buf = (char*)malloc(size);
    if (stream->buf == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    stream->size = size;
    stream->pos = 0;
    return stream;
}


void
stream_write(struct MemStream *stream, const char *format, ...)
{
    size_t format_len = strlen(format);
    char *non_null_format = malloc(format);

    /* TODO: format is null-terminated, may lead to misleading printf output */
    va_list args;
    va_start(args, format);
    int size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    char *src = malloc(size);
    if (src == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "malloc failed");

    va_start(args, format);
    vsnprintf(src, size, format, args);
    va_end(args);

    if (stream->pos + size > stream->size)
    {
        stream->buf = (char*)realloc(stream->buf, stream->pos + size);
        if (stream->buf == NULL)
            fail(ERROR_MEMORY_ALLOCATION, "realloc failed");
        stream->size = stream->pos + size;
    }
    memcpy(stream->buf + stream->pos, src, size);
    free(src);
    stream->pos += size;
}

void
free_stream(struct MemStream *stream)
{
    free(stream->buf);
    free(stream);
}

void
stream_write_to_file(struct MemStream *stream, const char *path)
{
    FILE* file = fopen(path, "wb");
	if (file == NULL) {
		perror(NULL);
		exit(-1);
	}

    for (size_t i = 0; i < stream->size; i++)
    {
        fprintf(file, "%c", stream->buf[i]);
    }

	fclose(file);
}
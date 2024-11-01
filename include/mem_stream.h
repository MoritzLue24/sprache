// TODO: memory allocation error checking, continue

#ifndef MEM_STREAM_H
#define MEM_STREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct MemStream
{
    char *buf;
    size_t size;
    size_t pos;
};

struct MemStream*
mem_open(size_t size)
{
    MemStream *stream = (MemStream*)malloc(sizeof(MemStream));
    stream->buf = (char*)malloc(size);
    stream->size = size;
    stream->pos = 0;
    return stream;
}

void
mem_write(struct MemStream *stream, const char *src, size_t size)
{
    if (stream->pos + size > stream->size)
    {
        stream->buf = (char*)realloc(stream->buf, stream->pos + size);
        stream->size = stream->pos + size;
    }
    memcpy(stream->buf + stream->pos, src, size);
    stream->pos += size;
}

#endif
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
stream_open(size_t size);

void
stream_write(struct MemStream *stream, const char *source, ...);

void
free_stream(struct MemStream *stream);

void
stream_write_to_file(struct MemStream *stream, const char *path);

void
print_stream(struct MemStream *stream);

#endif
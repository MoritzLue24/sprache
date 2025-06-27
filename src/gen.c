#include "gen.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "mem_stream.h"


struct MemStream *
gen(struct Node root)
{
    if (root.kind != NODE_ROOT)
        fail(ERROR_NODE_INVALID, "root node expected");
    struct MemStream *stream = stream_open(0);
    /* linux only */
    /* FILE *stream = fmemopen(buffer, size, "w"); */

    stream_write(stream, "section .text\nglobal _start\n_start:\n\tmov eax, 1\n\tmov ebx, 42\n\tint 0x80");
    return stream;
}

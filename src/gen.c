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

    stream_write(stream, "section .text\nglobal _main\n");
    /* stream_write(stream, "_main:\n\tmov eax, 42\n\tret\n"); */
    gen_node_list(root.n_root.body, stream);

    stream_write_to_file(stream, "gen_test.asm");
    free_stream(stream);

    return stream;
}

void
gen_node_list(struct NodeListElement *current, struct MemStream *stream)
{
    switch (current->self->kind)
    {
        case NODE_FUNCTION:
            gen_function(current->self->n_function, stream);
            break;

        default:
            break;
            /* fail(ERROR_NOT_IMPLEMENTED, NULL); */
    }
}

void
gen_function(struct Node_Function function, struct MemStream *stream)
{
    stream_write(stream, "_%s:", function.name.value);
    gen_node_list(function.body, stream);
}

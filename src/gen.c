#include "gen.h"
#include "error.h"

#include <stdio.h>
#include <string.h>


void
gen(struct Node root, char *const buffer, uint64_t size)
{
    memset(buffer, 0, size);
    if (root.kind != NODE_ROOT)
        fail(ERROR_NODE_INVALID, "root node expected");

    /* linux only */
    /* FILE *stream = fmemopen(buffer, size, "w"); */
    FILE *stream = NULL;
    if (stream == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "fmemopen failed");

    /* gen_node_list(root.n_root.body, stream); */
    fputs("section .text\nglobal _main\n_main:\n\tmov eax, 4\n\tret\n", stream);

    fflush(stream);
    fclose(stream);
}

void
gen_node_list(struct NodeListElement *current, FILE *stream)
{
    
}
#include "gen.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "utils.h"


char *
gen(struct Node node)
{
#ifndef _GNU_SOURCE
#error "gen.c requires _GNU_SOURCE to use open_memstream"
#endif

    char *buf = NULL;
    size_t size = 0;
    FILE *stream = open_memstream(&buf, &size);
    if (stream == NULL)
        fail(ERROR_MEMORY_ALLOCATION, "open_memstream failed");
    
    fprintf(stream, "section .text\nglobal _start\n");
    gen_block(stream, node);
    fclose(stream);
    return buf;
}

void
gen_block(FILE *stream, struct Node block)
{
    if (block.kind != NODE_BLOCK)
        fail(ERROR_NODE_INVALID, "gen_block only accepts NODE_BLOCK");
    for (unsigned int i = 0; i < block.n_block.count; i++)
    {
        struct Node statement = block.n_block.body[i];
        switch (statement.kind)
        {
            case NODE_FUNCTION:
                gen_function(stream, statement);
                break;
            case NODE_RETURN:
                gen_return(stream, statement);
                break;
            default:
                fail(ERROR_NODE_INVALID, "gen not yet implemented for kind %i", statement.kind);
                break;
        }
    }
}

void
gen_function(FILE *stream, struct Node function)
{
    if (function.kind != NODE_FUNCTION)
        fail(ERROR_NODE_INVALID, "gen_function only accepts NODE_FUNCTION");
    else if (function.n_function.name_token.type != TT_IDENTIFIER)
        fail(ERROR_NODE_INVALID, "gen_function only accepts identifier as name");

    fprintf(stream, "%s:\n", function.n_function.name_token.value);
    gen_block(stream, *function.n_function.block_node);
}

void
gen_return(FILE *stream, struct Node ret)
{
    if (ret.kind != NODE_RETURN)
        fail(ERROR_NODE_INVALID, "gen_return only accepts NODE_RETURN");
    else if (ret.n_return.value_node->kind != NODE_LITERAL)
        fail(ERROR_NODE_INVALID, "gen_return only accepts NODE_LITERAL as return value");
    else if (ret.n_return.value_node->n_literal.value_token.type != TT_LITERAL)
        fail(ERROR_NODE_INVALID, "gen_return only accepts literal token as return value");

    fprintf(stream, "\tmov eax, 1\n\tmov ebx, %s\n\tint 0x80\n", ret.n_return.value_node->n_literal.value_token.value);
}

#include "gen.h"

#include <stdio.h>
#include <stddef.h>

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
    gen_module(stream, node);
    fclose(stream);
    return buf;
}

void
gen_module(FILE *stream, struct Node module)
{
    if (module.kind != NODE_MODULE)
        fail(ERROR_NODE_INVALID, "gen_module only accepts NODE_MODULE");
    for (size_t i = 0; i < module.n_module.count; i++)
    {
        struct Node function = module.n_module.body[i];
        if (function.kind != NODE_FUNC_DEF)
            fail(ERROR_NODE_INVALID, "gen_module only accepts NODE_FUNC_DEF as body items");
        gen_function(stream, function);
    }
}

void
gen_function(FILE *stream, struct Node function)
{
    if (function.kind != NODE_FUNC_DEF)
        fail(ERROR_NODE_INVALID, "gen_function only accepts NODE_FUNC_DEF");
    else if (function.n_func_def.name_t.type != TT_IDENTIFIER)
        fail(ERROR_NODE_INVALID, "gen_function only accepts identifier as name");
    else if (function.n_func_def.name_t.value == NULL)
        fail(ERROR_NODE_INVALID, "function name token has no value");
        
    fprintf(stream, "%s:\n", function.n_func_def.name_t.value);
    gen_block(stream, *function.n_func_def.block_n);
}

void
gen_block(FILE *stream, struct Node block)
{
    if (block.kind != NODE_BLOCK)
        fail(ERROR_NODE_INVALID, "gen_block only accepts NODE_BLOCK");
    for (size_t i = 0; i < block.n_block.count; i++)
        gen_statement(stream, block.n_block.body[i]);
}

void
gen_statement(FILE *stream, struct Node statement)
{
    switch (statement.kind)
    {
        case NODE_RETURN:
            gen_return(stream, statement);
            break;
        default:
            fail(ERROR_NODE_INVALID, "invalid statement %i", statement.kind);
            break;
    }
}

void
gen_return(FILE *stream, struct Node ret)
{
    if (ret.kind != NODE_RETURN)
        fail(ERROR_NODE_INVALID, "gen_return only accepts NODE_RETURN");
    else if (ret.n_return.value_n->kind != NODE_LITERAL)
        fail(ERROR_NODE_INVALID, "gen_return only accepts NODE_LITERAL as return value");
    else if (ret.n_return.value_n->n_literal.value_t.type != TT_LITERAL)
        fail(ERROR_NODE_INVALID, "gen_return only accepts literal token as return value");

    fprintf(stream, "\tmov eax, 1\n\tmov ebx, %s\n\tint 0x80\n", ret.n_return.value_n->n_literal.value_t.value);
}

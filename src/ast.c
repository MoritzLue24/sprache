#include "ast.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "error.h"


void
print_node(struct Node node, uint8_t identation)
{
    char ident[identation * 2 + 1];
    for (size_t i = 0; i < identation * 2; ++i)
        ident[i] = ' ';
    ident[identation * 2] = '\0';

    switch (node.kind)
    {
        case NODE_MODULE:
            printf("%sMODULE\n", ident);
            for (size_t i = 0; i < node.n_module.count; ++i)
                print_node(node.n_module.body[i], identation + 1);
            break;
        case NODE_FUNC_DEF:
            printf("%sFUNCTION %s\n", ident, node.n_func_def.name_t.value);
            print_node(*node.n_func_def.block_n, identation + 1);
            break;

        case NODE_BLOCK:
            printf("%s{\n", ident);
            for (size_t i = 0; i < node.n_block.count; ++i)
                print_node(node.n_block.body[i], identation + 1);
            printf("%s}\n", ident);
            break;

        case NODE_RETURN:
            printf("%sRETURN \n", ident);
            print_node(*node.n_return.value_n, identation + 1);
            break;

        case NODE_LITERAL:
            printf("%sLITERAL %s\n", ident, node.n_literal.value_t.value);
            break;
    
        default:
            fail(ERROR_NODE_INVALID, "not imlpemented for print_node");
    }
}

void
free_node(struct Node node)
{
    switch (node.kind)
    {
        case NODE_MODULE:
            for (size_t i = 0; i < node.n_module.count; ++i)
                free_node(node.n_module.body[i]);
            free(node.n_module.body);
            break;

        case NODE_FUNC_DEF:
            if (node.n_func_def.name_t.value != NULL)
                free(node.n_func_def.name_t.value);
            free_node(*node.n_func_def.block_n);
            free(node.n_func_def.block_n);
            break;

        case NODE_BLOCK:
            for (size_t i = 0; i < node.n_block.count; ++i)
                free_node(node.n_block.body[i]);
            free(node.n_block.body);
            break;

        case NODE_RETURN:
            free_node(*node.n_return.value_n);
			free(node.n_return.value_n);
            break;

        case NODE_LITERAL:
            if (node.n_literal.value_t.value != NULL)
                free(node.n_literal.value_t.value);
            break;

        default:
            fail(ERROR_NODE_INVALID, "not imlpemented for free_node");
    }
}
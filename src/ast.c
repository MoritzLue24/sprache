#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "error.h"


void
print_node(struct Node node, uint8_t identation)
{
    char ident[identation * 2 + 1];
    for (uint8_t i = 0; i < identation * 2; ++i)
        ident[i] = ' ';
    ident[identation * 2] = '\0';

    switch (node.kind)
    {
        case NODE_BLOCK:
            printf("%s{\n", ident);
            for (uint32_t i = 0; i < node.n_block.count; ++i)
                print_node(node.n_block.body[i], identation + 1);
            printf("%s}\n", ident);
            break;

        case NODE_FUNCTION:
            printf("%sFUNCTION %s\n", ident, node.n_function.name_token.value);
            print_node(*node.n_function.block_node, identation + 1);
            break;

        case NODE_RETURN:
            printf("%sRETURN \n", ident);
            print_node(*node.n_return.value_node, identation + 1);
            break;

        case NODE_LITERAL:
            printf("%sLITERAL %s\n", ident, node.n_literal.value_token.value);
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
        case NODE_BLOCK:
            for (uint32_t i = 0; i < node.n_block.count; ++i)
                free_node(node.n_block.body[i]);
            free(node.n_block.body);
            break;

        case NODE_FUNCTION:
            if (node.n_function.name_token.value != NULL)
                free(node.n_function.name_token.value);
            free_node(*node.n_function.block_node);
            free(node.n_function.block_node);
            break;

        case NODE_RETURN:
            free_node(*node.n_return.value_node);
			free(node.n_return.value_node);
            break;

        case NODE_LITERAL:
            if (node.n_literal.value_token.value != NULL)
                free(node.n_literal.value_token.value);
            break;

        default:
            fail(ERROR_NODE_INVALID, "not imlpemented for free_node");
    }
}
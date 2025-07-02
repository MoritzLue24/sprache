#include "gen.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "utils.h"


char *
gen(struct Node_Root root)
{
    char *stream = NULL;
    /* linux only */
    /* FILE *stream = fmemopen(buffer, size, "w"); */

    //stream_write(stream, "section .text\nglobal _start\n_start:\n\tmov eax, 1\n\tmov ebx, %s\n\tint 0x80", root.n_root.body->self->n_function.body->self->n_return.value->n_literal.value.value);
    append_string(&stream, "section .text\nglobal _start\n");
    gen_node_list(&stream, root.body);
    return stream;
}

void
gen_node_list(char **stream, struct NodeListElement *node_list)
{
    struct NodeListElement *current = node_list;
	while (current != NULL && current->self != NULL)
	{
        switch (current->self->kind)
        {
            case NODE_FUNCTION:
                gen_function(stream, current->self->n_function);
                break;
            case NODE_RETURN:
                gen_return(stream, current->self->n_return);
                break;
            default:
                // TODO
                break;
        }
		current = current->next;
	}
}

void
gen_function(char **stream, struct Node_Function function)
{
    if (!strcmp(function.name.value, "main"))
        append_string(stream, "_start:\n");
    else
        append_string(stream, "%s:\n", function.name.value);
    gen_node_list(stream, function.body);
}

void
gen_return(char **stream, struct Node_Return ret)
{
    append_string(stream, "\tmov eax, 1\n\tmov ebx, %s\n\tint 0x80\n", ret.value->n_literal.value.value);
}
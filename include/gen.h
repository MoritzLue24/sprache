#ifndef GEN_H
#define GEN_H

#include "ast.h"
#include "mem_stream.h"

#include <stdio.h>


struct MemStream *
gen(struct Node_Root root);

void
gen_node_list(struct MemStream *stream, struct NodeListElement *node_list);

void
gen_function(struct MemStream *stream, struct Node_Function function);

void
gen_return(struct MemStream *stream, struct Node_Return ret);

#endif
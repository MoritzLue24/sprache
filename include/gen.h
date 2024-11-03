#ifndef GEN_H
#define GEN_H

#include "ast.h"
#include "mem_stream.h"

#include <stdio.h>


struct MemStream *
gen(struct Node root);

void
gen_node_list(struct NodeListElement *current, struct MemStream *stream);

void
gen_function(struct Node_Function function, struct MemStream *stream);

#endif
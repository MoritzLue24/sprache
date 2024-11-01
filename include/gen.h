#ifndef GEN_H
#define GEN_H

#include "ast.h"

#include <stdio.h>


void
gen(struct Node root, char *const buffer, uint64_t size);

void
gen_node_list(struct NodeListElement *current, FILE *stream);

#endif
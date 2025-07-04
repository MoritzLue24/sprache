#ifndef GEN_H
#define GEN_H

#include <stdio.h>

#include "ast.h"


char *
gen(struct Node_Root root);

void
gen_node_list(char **stream, struct NodeListElement *node_list);

void
gen_function(char **stream, struct Node_Function function);

void
gen_return(char **stream, struct Node_Return ret);

#endif
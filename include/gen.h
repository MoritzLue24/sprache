#ifndef GEN_H
#define GEN_H

#include <stdio.h>

#include "ast.h"


char *
gen(struct Node node);

void
gen_module(FILE *stream, struct Node module);

void
gen_function(FILE *stream, struct Node function);

void
gen_block(FILE *stream, struct Node block);

void
gen_statement(FILE *stream, struct Node statement);

void
gen_return(FILE *stream, struct Node ret);


#endif
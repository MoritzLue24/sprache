#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"


char*
read_file(const char *path);

bool
is_digit(char c);

bool
is_ident_start(char c);

bool
is_ident_char(char c);

void
print_node(struct Node node, uint8_t identation);

void
print_node_list(struct NodeListElement *root, uint8_t identation);

void
append_node_list(struct NodeListElement *root, struct Node value);

void
insert_node_list(struct NodeListElement *root, struct Node value, uint32_t nth);

void
free_node_list(struct NodeListElement *root);

#endif /* UTILS_H */

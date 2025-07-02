#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"
#include "tokens.h"


char*
read_file(const char *path);

bool
is_digit(char c);

bool
is_ident_start(char c);

bool
is_ident_char(char c);

char*
format(const char *source, ...);

void
skip_whitespace(struct Loc *loc);

void
print_node(struct Node node, uint8_t identation);

void
free_token(struct Token token);

void
free_node(struct Node node);

void
print_node_list(struct NodeListElement *root, uint8_t identation);

void
free_node_list(struct NodeListElement *root);

void
append_node_list(struct NodeListElement *root, struct Node value);

void
insert_node_list(struct NodeListElement *root, struct Node value, uint32_t nth);

/* `dest` cannot be initialized with a string literal.
 * string literals create read-only memory.
*/
void
append_string(char **dest, const char *source, ...);

#endif /* UTILS_H */

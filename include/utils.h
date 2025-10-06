#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "ast.h"
#include "tokens.h"


char*
read_file(const char *path);

char*
format(const char *source, ...);

void
skip_whitespace(struct Loc *loc);

void
print_node(struct Node node, uint8_t identation);

/* `dest` cannot be initialized with a string literal.
 * string literals create read-only memory.
*/
char *
append_string(char *dest, const char *source, ...);

#endif /* UTILS_H */

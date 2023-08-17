#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>


char*
read_file(const char *path);

bool
is_digit(char c);

bool
is_ident_start(char c);

bool
is_ident_char(char c);

void
skip_whitespaces(const char *source, int32_t *i);

#endif /* UTILS_H */

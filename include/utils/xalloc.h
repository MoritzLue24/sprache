#ifndef XALLOC_H
#define XALLOC_H

#include <stdlib.h>


void* xmalloc(size_t size);

void* xcalloc(size_t n, size_t size);

void* xrealloc(void* ptr, size_t n, size_t size);

char* xstrdup(const char* str);

void xfree(void** p);

#endif
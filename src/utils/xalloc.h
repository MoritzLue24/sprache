#ifndef XALLOC_H
#define XALLOC_H

#include <stddef.h>

void* xmalloc(size_t size);
void* xcalloc(size_t count, size_t size);
void* xrealloc(void* p, size_t size);

#endif

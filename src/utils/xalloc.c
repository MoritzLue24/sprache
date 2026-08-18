#include "utils/xalloc.h"

#include <stdio.h>
#include <stdlib.h>

static void xalloc_fail(void)
{
    fprintf(stderr, "out of memory\n");
    exit(1);
}

void* xmalloc(size_t size)
{
    void* p = malloc(size);
    if (!p && size)
        xalloc_fail();
    return p;
}

void* xcalloc(size_t count, size_t size)
{
    void* p = calloc(count, size);
    if (!p && count && size)
        xalloc_fail();
    return p;
}

void* xrealloc(void* p, size_t size)
{
    void* new_p = realloc(p, size);
    if (!new_p && size)
        xalloc_fail();
    return new_p;
}

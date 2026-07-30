#include "utils/xalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void* xmalloc(size_t size)
{
    void* p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Fatal: out of memory (malloc(%zu) failed)\n", size);
        exit(1);
    }
    return p;
}

void* xcalloc(size_t n, size_t size)
{
    void* p = calloc(n, size);
    if (p == NULL) {
        fprintf(stderr, "Fatal: out of memory (calloc(%zu, %zu) failed)\n", n, size);
        exit(1);
    }
    return p;
}

void* xrealloc(void* ptr, size_t n, size_t size)
{
    void* p = realloc(ptr, n * size);
    if (p == NULL) {
        fprintf(stderr, "Fatal: out of memory (realloc(%p, %zu) failed)\n", ptr, size);
        exit(1);
    }
    return p;
}

char* xstrdup(const char* str)
{
    if (str == NULL)
        return NULL;

    size_t len = strlen(str) + 1;
    char* copy = xmalloc(len);

    memcpy(copy, str, len);
    return copy;
}

void xfree(void** p)
{
    free(*p);
    *p = NULL;
}
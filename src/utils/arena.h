#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

struct Arena {

};

void arena_init(struct Arena* a);
void arena_free(struct Arena* a);

void* arena_calloc(struct Arena* a, size_t size);

#define ARENA_CALLOC(a, T) 

#endif
#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

struct ArenaBlock;

struct Arena {
    struct ArenaBlock* head;
    struct ArenaBlock* current;
};

void init_arena(struct Arena* a);
void free_arena(struct Arena* a);

void* arena_calloc(struct Arena* a, size_t size, size_t align);

#define ARENA_CALLOC(a, T) ((T*)arena_calloc((a), sizeof(T), _Alignof(T)))
#define ARENA_CALLOC_LIST(a, n, T) \
    ((T*)arena_calloc((a), (n) * sizeof(T), _Alignof(T)))

#endif

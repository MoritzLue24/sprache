#include "utils/arena.h"
#include "utils/xalloc.h"
#include <stdlib.h>
#include <string.h>

#define ARENA_BLOCK_SIZE ((size_t)4096)

struct ArenaBlock {
    struct ArenaBlock* next;
    size_t             capacity;
    size_t             used;
    unsigned char      data[];
};

static size_t align_up(size_t n, size_t align)
{
    return (n + align - 1) & ~(align - 1);
}

static struct ArenaBlock* arena_block_new(size_t capacity)
{
    struct ArenaBlock* block = xmalloc(sizeof(struct ArenaBlock) + capacity);\
    block->next     = NULL;
    block->capacity = capacity;
    block->used     = 0;
    return block;
}

void init_arena(struct Arena* a)
{
    a->head    = NULL;
    a->current = NULL;
}

void free_arena(struct Arena* a)
{
    struct ArenaBlock* block = a->head;
    while (block) {
        struct ArenaBlock* next = block->next;
        free(block);
        block = next;
    }
    a->head    = NULL;
    a->current = NULL;
}

void* arena_calloc(struct Arena* a, size_t size, size_t align)
{
    size_t offset = a->current ? align_up(a->current->used, align) : 0;

    if (!a->current || offset + size > a->current->capacity) {
        size_t capacity = size > ARENA_BLOCK_SIZE ? size : ARENA_BLOCK_SIZE;
        struct ArenaBlock* block = arena_block_new(capacity);

        if (a->current)
            a->current->next = block;
        else
            a->head = block;
        a->current = block;
        offset = 0;
    }

    void* ptr = a->current->data + offset;
    a->current->used = offset + size;
    memset(ptr, 0, size);
    return ptr;
}

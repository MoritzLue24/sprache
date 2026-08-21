#ifndef DARRAY_H
#define DARRAY_H

#include "utils/arena.h"
#include <string.h>

#define DARRAY_INIT(arena, list, init_cap) do { \
    (list)->items = ARENA_CALLOC_LIST( \
        (arena), (init_cap), __typeof__(*(list)->items) \
    ); \
    (list)->count = 0; \
    (list)->capacity = (init_cap); \
} while (0)

#define DARRAY_ENSURE(arena, list) do { \
    if ((list)->count >= (list)->capacity) { \
        size_t new_cap_ = (list)->capacity * 2; \
        __typeof__(*(list)->items)* new_items_ = ARENA_CALLOC_LIST( \
            (arena), new_cap_, __typeof__(*(list)->items) \
        ); \
        memcpy( \
            new_items_, (list)->items, \
            (list)->count * sizeof(*(list)->items) \
        ); \
        (list)->items = new_items_; \
        (list)->capacity = new_cap_; \
    } \
} while (0)

#define DARRAY_ADD(arena, list, element) do { \
    DARRAY_ENSURE((arena), (list)); \
    (list)->items[(list)->count++] = (element); \
} while (0)

#endif

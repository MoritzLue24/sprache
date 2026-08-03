#include "backend/ir/stack_frame.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "utils/xalloc.h"


/// @brief next stackframe offset
static unsigned int sf_next_offset = 1;

static void expand(struct StackFrame* sf)
{
    sf->capacity *= 2;
    sf->entries = xrealloc(sf->entries, sf->capacity, sizeof(struct SFEntry));
}

void init_stackframe(struct StackFrame* sf, size_t capacity)
{
    sf_next_offset = 1;
    sf->entries = xcalloc(capacity, sizeof(struct SFEntry));
    sf->size = 0;
    sf->capacity = capacity;
}

unsigned int stackframe_push(struct StackFrame* sf, const struct Symbol* sym)
{
    assert(sym != NULL);

    assert(!get_sf_offset(sf, sym, NULL));

    if (sf->size >= sf->capacity) {
        expand(sf);
    }

    sf->entries[sf->size++] = (struct SFEntry){
        .symbol = sym,
        .offset = sf_next_offset++
    };
    return sf_next_offset - 1;
}

bool get_sf_offset(const struct StackFrame* sf, const struct Symbol* sym, unsigned int* out_sf_offset)
{
    assert(sym != NULL);
    for (size_t i = 0; i < sf->size; i++) {
        if (sf->entries[i].symbol == sym) {
            if (out_sf_offset != NULL) {
                *out_sf_offset = sf->entries[i].offset;
            }
            return true;
        }
    }
    return false; 
}

unsigned int get_stacksize(const struct StackFrame* sf)
{
    unsigned int s_size = 0;
    for (size_t i = 0; i < sf->size; i++) {
        if (s_size < sf->entries[i].offset + 1) {
            s_size = sf->entries[i].offset + 1;
        }
    }
    return s_size;
}

void free_stackframe(struct StackFrame* sf)
{
    xfree((void**)&sf->entries);
}

void print_stackframe(const char* label, const struct StackFrame* sf, int depth)
{
    printf("%*s", depth * 4, "");
    if (label != NULL) {
        printf("%s: [\n", label);
    }

    for (size_t i = 0; i < sf->size; i++) {
        printf("%*s", (depth + 1) * 4, "");
        printf("SFEntry(\n");
        print_symbol("symbol", sf->entries[i].symbol, depth + 2);
        printf("%*s", (depth + 2) * 4, "");
        printf("offset: '%i'\n", sf->entries[i].offset);
        printf("%*s", (depth + 1) * 4, "");
        printf(")\n");
    }

    printf("%*s", depth * 4, "");
    printf("]\n");
}
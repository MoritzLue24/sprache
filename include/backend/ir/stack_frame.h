#ifndef STACK_FRAME_H
#define STACK_FRAME_H

#include <stdlib.h>

#include "frontend/sema/symbols.h"


struct SFEntry {
    const struct Symbol* symbol;
    unsigned int offset;
};

struct StackFrame {
    struct SFEntry* entries;
    size_t size;
    size_t capacity;
};


void init_stackframe(struct StackFrame* sf, size_t capacity);

unsigned int stackframe_push(struct StackFrame* sf, const struct Symbol* sym);

bool get_sf_offset(const struct StackFrame* sf, const struct Symbol* sym, unsigned int* out_sf_offset);

unsigned int get_stacksize(const struct StackFrame* sf);

void free_stackframe(struct StackFrame* sf);

void print_stackframe(const char* label, const struct StackFrame* sf, int depth);


#endif
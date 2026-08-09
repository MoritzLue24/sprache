#ifndef STACK_FRAME_H
#define STACK_FRAME_H

#include <stdlib.h>

#include "frontend/sema/symbols.h"

#define STACKFRAME_INIT_CAPACITY 10


struct SFEntry {
    const struct Symbol* symbol;
    bool needs_resolving;

    unsigned int offset;
    unsigned int rel_arg_offset;
};

struct StackFrame {
    struct SFEntry* entries;
    /// @brief Stack-size: the actual size that needs allocation
    /// (parameters not incuded, they live ouside the Sf).
    /// Gets incremented automatically
    size_t s_size;
    /// @brief Actual size of the `StackFrame.entries` list
    size_t size;
    size_t capacity;
};


void init_stackframe(struct StackFrame* sf);

void free_stackframe(struct StackFrame* sf);

void use_stackframe(struct StackFrame* sf);

void unuse_stackframe(const struct StackFrame* sf);

struct SFEntry* stackframe_push(const struct Symbol* sym, bool is_arg);

struct SFEntry* stackframe_lookup(const struct Symbol* sym);

void resolve_arg_offsets();

void print_stackframe(const struct StackFrame* sf, const char* label, int depth);


#endif
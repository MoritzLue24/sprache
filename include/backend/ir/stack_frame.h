#ifndef STACK_FRAME_H
#define STACK_FRAME_H

#include <stdlib.h>

#include "frontend/sema/symbols.h"


struct SFEntry {
    const struct Symbol* symbol;
    bool needs_resolving;

    unsigned int offset;
    unsigned int rel_arg_offset;

    struct SFEntry* next;
};

struct StackFrame {
    char* func_ident;
    struct SFEntry* head;
    struct SFEntry* tail;
    /// @brief Stack-size: the actual size that needs allocation
    /// (parameters not incuded, they live ouside the Sf).
    /// Gets incremented automatically
    size_t s_size;
};


void init_stackframe(struct StackFrame* sf, const char* func_ident);

void free_stackframe(struct StackFrame* sf);

void use_stackframe(struct StackFrame* sf);

void unuse_stackframe(const struct StackFrame* sf);

struct SFEntry* stackframe_push(const struct Symbol* sym, bool is_arg);

struct SFEntry* stackframe_lookup(const struct Symbol* sym);

void resolve_arg_offsets();

const char* get_func_ident();

void print_stackframe(const struct StackFrame* sf, const char* label, int depth);


#endif
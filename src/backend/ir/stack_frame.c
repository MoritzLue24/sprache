#include "backend/ir/stack_frame.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "utils/xalloc.h"
#include "backend/target/avr_target.h"


static struct StackFrame* ctx = NULL;
/// @brief next stackframe offset
static unsigned int sf_next_offset = 1;
/// @brief next arg offset relative to the BP + ret_addr_size
static unsigned int next_rel_arg_offset = 1;


static void expand(struct StackFrame* sf)
{
    sf->capacity *= 2;
    sf->entries = xrealloc(sf->entries, sf->capacity, sizeof(struct SFEntry));
}

void init_stackframe(struct StackFrame* sf)
{
    sf_next_offset = 1;
    sf->entries = xcalloc(STACKFRAME_INIT_CAPACITY, sizeof(struct SFEntry));
    sf->size = 0;
    sf->capacity = STACKFRAME_INIT_CAPACITY;
}

void free_stackframe(struct StackFrame* sf)
{
    xfree((void**)&sf->entries);
}

void use_stackframe(struct StackFrame* sf)
{
    ctx = sf;
}

void unuse_stackframe(const struct StackFrame* sf)
{
    assert(sf == ctx);
    ctx = NULL;
}

struct SFEntry* stackframe_push(const struct Symbol* sym, bool is_arg)
{
    assert(ctx);
    assert(!stackframe_lookup(sym));

    if (ctx->size >= ctx->capacity) {
        expand(ctx);
    }

    if (is_arg) {
        ctx->entries[ctx->size++] = (struct SFEntry){
            .symbol = sym,
            .needs_resolving = true,
            .rel_arg_offset = next_rel_arg_offset++
        };
        return &ctx->entries[ctx->size - 1];
    }
    ctx->entries[ctx->size++] = (struct SFEntry){
        .symbol = sym,
        .needs_resolving = false,
        .offset = sf_next_offset++
    };
    ctx->s_size++;
    return &ctx->entries[ctx->size - 1];
}

struct SFEntry* stackframe_lookup(const struct Symbol* sym)
{
    assert(ctx);
    for (size_t i = 0; i < ctx->size; i++) {
        if (ctx->entries[i].symbol == sym) {
            return &ctx->entries[i];
        }
    }
    return NULL;
}

void resolve_arg_offsets()
{
    for (size_t i = 0; i < ctx->size; i++) {
        if (!ctx->entries[i].needs_resolving)
            continue;
        ctx->entries[i].offset = ctx->s_size
            + target.sp_size_bytes
            + target.ret_addr_size_bytes
            + ctx->entries[i].rel_arg_offset;
        ctx->entries[i].needs_resolving = false;
    }
}

void print_stackframe(const struct StackFrame* sf, const char* label, int depth)
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
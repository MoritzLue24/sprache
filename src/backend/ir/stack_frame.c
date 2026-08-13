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


void init_stackframe(struct StackFrame* sf, const char* func_ident)
{
    sf_next_offset = 1;
    next_rel_arg_offset = 1;
    sf->func_ident = xstrdup(func_ident);
    sf->head = NULL;
    sf->tail = NULL;
    sf->s_size = 0;
}

static void free_sfentry_rec(struct SFEntry* head)
{
    if (head == NULL) {
        return;
    }
    free_sfentry_rec(head->next);
    xfree((void**)&head);
}

void free_stackframe(struct StackFrame* sf)
{
    xfree((void**)&sf->func_ident);
    free_sfentry_rec(sf->head);
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

    struct SFEntry* entry = xmalloc(sizeof(struct SFEntry));
    if (is_arg) {
        *entry = (struct SFEntry){
            .symbol = sym,
            .needs_resolving = true,
            .rel_arg_offset = next_rel_arg_offset++
        };
    }
    else {
        *entry = (struct SFEntry){
            .symbol = sym,
            .needs_resolving = false,
            .offset = sf_next_offset++
        };
        ctx->s_size++;
    }

    if (ctx->head == NULL) {
        ctx->head = entry;
    }
    else {
        ctx->tail->next = entry;
    }
    ctx->tail = entry;
    return entry;
}

struct SFEntry* stackframe_lookup(const struct Symbol* sym)
{
    assert(ctx);
    for (struct SFEntry* e = ctx->head; e != NULL; e = e->next) {
        if (e->symbol == sym) {
            return e;
        }
    }
    return NULL;
}

void resolve_arg_offsets()
{
    for (struct SFEntry* e = ctx->head; e != NULL; e = e->next) {
        if (!e->needs_resolving)
            continue;
        e->offset = ctx->s_size
            + target.sp_size_bytes
            + target.ret_addr_size_bytes
            + e->rel_arg_offset;
        e->needs_resolving = false;
    }
}

const char* get_func_ident()
{
    assert(ctx);
    return ctx->func_ident;
}

void print_stackframe(const struct StackFrame* sf, const char* label, int depth)
{
    printf("%*s", depth * 4, "");
    if (label != NULL) {
        printf("%s: [\n", label);
    }
    
    for (const struct SFEntry* e = sf->head; e != NULL; e = e->next) {
        printf("%*s", (depth + 1) * 4, "");
        printf("SFEntry(\n");
        print_symbol("symbol", e->symbol, depth + 2);
        printf("%*s", (depth + 2) * 4, "");
        printf("offset: '%i'\n", e->offset);
        printf("%*s", (depth + 1) * 4, "");
        printf(")\n");
    }

    printf("%*s", depth * 4, "");
    printf("]\n");
}
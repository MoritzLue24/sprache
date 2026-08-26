#include "sema/symbols.h"
#include "utils/arena.h"
#include "utils/darray.h"
#include <string.h>
#include <assert.h>

void symtable_init(struct SymTable* st)
{
    st->current = NULL;
}

void symtable_enter_scope(struct Arena* a, struct SymTable* st)
{
    struct Scope* s = ARENA_CALLOC(a, struct Scope);
    DARRAY_INIT(a, &s->syms, SYMBOLLIST_INIT_CAPACITY);
    s->parent = st->current;
    st->current = s;
}

void symtable_exit_scope(struct SymTable* st)
{
    assert(st->current);
    st->current = st->current->parent;
}

struct Symbol* symtable_declare(
    struct Arena* a, struct SymTable* st, struct Symbol sym
) {
    assert(st->current);
    if (symtable_lookup(st, sym.name) != NULL) return NULL;

    struct Symbol* sym_ptr = ARENA_CALLOC(a, struct Symbol);
    *sym_ptr = sym;
    DARRAY_ADD(a, &st->current->syms, sym_ptr);
    return sym_ptr;
}

struct Symbol* symtable_lookup(const struct SymTable* st, const char* name)
{
    for (const struct Scope* s = st->current; s != NULL; s = s->parent) {
        for (size_t i = 0; i < s->syms.count; i++) {
            if (strcmp(name, s->syms.items[i]->name) == 0) {
                return s->syms.items[i];
            }
        }
    }
    return NULL;
}
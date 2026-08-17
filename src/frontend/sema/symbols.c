#include "frontend/sema/symbols.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "utils/xalloc.h"


static void expand_scope(struct Scope* s)
{
    s->capacity *= 2;
    s->data = xrealloc(s->data, s->capacity, sizeof(struct Symbol*));
}

static void expand_arena(struct SymTable* st)
{
    st->arena_capacity *= 2;
    st->arena = xrealloc(st->arena, st->arena_capacity, sizeof(struct Symbol*));
}

static const char* symbol_type_str(enum SymbolType type)
{
    switch (type) {
        case SYM_FUNC: return "FUNC";
        case SYM_LOCAL: return "LOCAL";
        case SYM_TARGET: return "TARGET";
        default: assert(0);
    }
}

void print_symbol(const char* label, const struct Symbol* s, int depth)
{
    printf("%*s", depth * 4, "");
    if (label != NULL) {
        printf("%s: ", label);
    }

    if (s == NULL) {
        printf("<unresolved>\n");
        return;
    }

    printf("Symbol (\n");

    printf("%*s", (depth + 1) * 4, "");
    printf("type: '%s'\n", symbol_type_str(s->type));

    printf("%*s", (depth + 1) * 4, "");
    printf("ident: '%s'\n", s->ident);

    printf("%*s", (depth + 1) * 4, "");
    switch (s->type)
    {
        case SYM_FUNC:
            printf("loc_decl: %i:%i\n", s->loc_decl.ln, s->loc_decl.col);
            printf("%*s", (depth + 1) * 4, "");
            printf("func.param_count: %li\n", s->func.param_count);
            break;
        case SYM_LOCAL:
            printf("loc_decl: %i:%i\n", s->loc_decl.ln, s->loc_decl.col);
            break;
        case SYM_TARGET:
            printf("loc_decl: <target>\n");
            break;
        default:
            assert(0);
    }
    printf("%*s", depth * 4, "");
    printf(")\n");
}

void init_symtable(struct SymTable* st, size_t arena_capacity)
{
    st->current = NULL;
    st->arena = xcalloc(arena_capacity, sizeof(struct Symbol*));
    st->arena_size = 0;
    st->arena_capacity = arena_capacity;
}

void symtable_enter_scope(struct SymTable* st, size_t scope_capacity)
{
    struct Scope* s = xmalloc(sizeof(struct Scope));
    s->data = xcalloc(scope_capacity, sizeof(struct Symbol*));
    s->capacity = scope_capacity;
    s->size = 0;
    s->parent = st->current;

    st->current = s;
}

void symtable_exit_scope(struct SymTable* st)
{
    assert(st->current != NULL);
    struct Scope* s = st->current->parent;

    xfree((void**)&st->current->data);
    xfree((void**)&st->current);

    st->current = s;
}

const struct Symbol* symtable_lookup(const struct SymTable* st, const char* ident);
struct Symbol* symtable_declare(struct SymTable* st, enum SymbolType type, const char* ident, struct Loc loc_decl)
{
    assert(st->current != NULL);

    const struct Symbol* existing = symtable_lookup(st, ident);
    if (existing != NULL)
        return NULL;

    if (st->arena_size >= st->arena_capacity)
        expand_arena(st);

    if (st->current->size >= st->current->capacity)
        expand_scope(st->current);

    struct Symbol* sym = xmalloc(sizeof(struct Symbol));
    sym->type = type;
    sym->ident = xstrdup(ident);
    sym->loc_decl = loc_decl;

    st->arena[st->arena_size++] = sym;
    st->current->data[st->current->size++] = sym;

    return sym;
}

struct Symbol* symtable_declare_func(struct SymTable* st, const char* ident, struct Loc loc_decl, size_t param_count)
{
    struct Symbol* sym = symtable_declare(st, SYM_FUNC, ident, loc_decl);
    if (sym == NULL) {
        return NULL;
    }
    sym->func.param_count = param_count;
    return sym;

}

static const struct Symbol* scope_lookup_rec(const struct Scope* scope, const char* ident)
{
    assert(scope != NULL);

    for (size_t i = 0; i < scope->size; i++) {
        if (strcmp(scope->data[i]->ident, ident) == 0) {
            return scope->data[i];
        }
    }
    if (scope->parent != NULL)
        return scope_lookup_rec(scope->parent, ident);
    return NULL;
}

const struct Symbol* symtable_lookup(const struct SymTable* st, const char* ident)
{
    return scope_lookup_rec(st->current, ident);
}

void free_symtable(struct SymTable* st)
{
    while (st->current != NULL) {
        symtable_exit_scope(st);
    }

    for (size_t i = 0; i < st->arena_size; i++) {
        xfree((void**)&st->arena[i]->ident);
        xfree((void**)&st->arena[i]);
    }
    xfree((void**)&st->arena);
}

void add_redeclaration_error(struct ErrorList* errors, const struct Symbol* sym, struct Loc loc)
{
    switch (sym->type) {
        case SYM_FUNC:
        case SYM_LOCAL:
            add_error(
                errors, ERROR_REDECLARATION, loc,
                "Symbol '%s' already declared at line %i, column %i",
                sym->ident, sym->loc_decl.ln, sym->loc_decl.col
            );
            break;

        case SYM_TARGET:
            add_error(
                errors, ERROR_REDECLARATION, loc,
                "Symbol '%s' is a target constant",
                sym->ident
            );
            break;

        default:
            assert(0);
    }
}
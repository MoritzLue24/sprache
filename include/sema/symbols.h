#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdlib.h>
#include <stdbool.h>

#include "core/loc.h"


enum SymbolType {
    SYM_LOCAL,
    SYM_TARGET
};

struct Symbol {
    enum SymbolType type;
    char* ident;
    /// @brief symbols of type `SYM_TARGET` does not have locations
    struct Loc loc_decl;
};

struct Scope {
    /// @brief list of pointers, pointing to a symbol in the arena
    struct Symbol** data;
    size_t size;
    size_t capacity;
    struct Scope* parent;
};

struct SymTable {
    struct Scope* current;

    /// @brief Stores & Owns ALL symbols (as pointer so we can reallocate)
    struct Symbol** arena;
    size_t arena_size;
    size_t arena_capacity;
};

/// @brief prints the given symbol with the given depth as indentation, and with a label in front (NULL for nothing). Prints label: <unresolved> if s == NULL
void print_symbol(const char* label, const struct Symbol* s, int depth);

void init_symtable(struct SymTable* st, size_t arena_capacity);

void symtable_enter_scope(struct SymTable* st, size_t scope_capacity);

void symtable_exit_scope(struct SymTable* st);

/// @brief Declares a new symbol to the given table
/// @return NULL when the symbol already exists, pointer to created symbol on success
struct Symbol* symtable_declare(struct SymTable* st, enum SymbolType type, const char* ident, struct Loc loc_decl);

struct Symbol* symtable_lookup(const struct SymTable* st, const char* ident);

void free_symtable(struct SymTable* st);

#endif
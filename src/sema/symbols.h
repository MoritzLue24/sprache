#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "sprache/diag.h"
#include "type/type.h"
#include "builtin/builtin.h"
#include <stddef.h>
#include <stdio.h>

#define SYMBOLLIST_INIT_CAPACITY 10

enum SymbolKind {
    SYM_INVALID,
    SYM_TYPE,
    SYM_BUILTIN,
    SYM_FUNC,
    SYM_VAR,
};

/// @returns the symbol kind as a string.
/// On invalid kind (not SYM_INVALID), returns NULL.
const char* symbol_kind_str(enum SymbolKind sym_kind);

struct Symbol {
    enum SymbolKind kind;
    const char* name;
    enum TypeKind type;
    /// @brief SOURCE_LOC_NULL for everything predeclared
    struct SourceLoc loc_decl;

    /// @brief For SYM_BUILTIN and SYM_FUNC
    size_t param_count;
    /// @brief For SYM_BUILTIN and SYM_FUNC
    const enum TypeKind* params;
    /// @brief For SYM_BUILTIN
    enum BuiltinKind builtin_kind;
};

/// @brief Dumps a symbol to the given stream, indented with the given depth. 
void symbol_dump(
    const char* label, const struct Symbol* sym, FILE* out, int depth
);

struct SymbolList {
    struct Symbol** items;
    size_t count;
    size_t capacity;
};

struct Scope {
    struct SymbolList syms;
    struct Scope* parent;
};

struct SymTable {
    struct Scope* current;
};

struct Arena;

/// @brief Initialises the given symtable.
/// Does not enter a scope automatically.
void symtable_init(struct SymTable* st);
void symtable_enter_scope(struct Arena* a, struct SymTable* st);
/// @brief Exits the current scope.
/// Asserts, if there is no current scope.
void symtable_exit_scope(struct SymTable* st);

/// @brief Declares a new symbol to the current scope.
/// @returns Pointer to the declared symbol,
/// NULL if the symbol is already declared
struct Symbol* symtable_declare(
    struct Arena* a, struct SymTable* st, struct Symbol sym
);
/// @note Searches accross this scope & all upper scopes.
/// @returns The symbol with the supplied name, NULL on no matches.
struct Symbol* symtable_lookup(const struct SymTable* st, const char* name);

#endif
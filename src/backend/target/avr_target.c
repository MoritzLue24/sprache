#include "backend/target/avr_target.h"

#include <assert.h>


/// @note Owned by the preset, NOT by the symtable: `target_declare_symbols`
/// hands the idents to `symtable_declare`, which copies them into symbols of
/// its own. These entries must never end up in the symtable arena, or
/// `free_symtable` would try to free string literals.
static const struct Symbol atmega16_symbols[] = {
    {
        .type = SYM_TARGET,
        .ident = "PINA",
        .loc_decl = EMPTY_LOC
    }
};

const struct AVRTarget atmega16 = {
    .name = "ATmega16",
    // r0, r1 is used by `mul`
    .zero_reg = 2,
    .tmp_reg = 3,
    // registers used by operations (add, or, mul, and, etc.)
    .oprnd_reg_first = 16,
    .oprnd_reg_num = 4,
    .ret_reg = 24,

    .sp_size_bytes = 2,
    .ret_addr_size_bytes = 2,

    .symbol_count = sizeof(atmega16_symbols) / sizeof(atmega16_symbols[0]),
    .symbols = atmega16_symbols
};

struct AVRTarget target = atmega16;


void target_declare_symbols(struct SymTable* st)
{
    for (size_t i = 0; i < target.symbol_count; i++) {
        const struct Symbol* sym = &target.symbols[i];
        if (!symtable_declare(st, SYM_TARGET, sym->ident, EMPTY_LOC)) {
            assert(0);
        }
    }
}
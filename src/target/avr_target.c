#include "target/avr_target.h"

#include <assert.h>


const struct AVRTarget atmega16 = {
    .name = "ATmega16",
    // r0, r1 is used by `mul`
    .zero_reg = 2,
    .tmp_reg = 3,
    // registers used by operations (add, or, mul, and, etc.)
    .oprnd_reg_first = 16,
    .oprnd_reg_num = 4,
    .ret_reg = 24,

    .symbol_count = 1,
    .symbols = {
        {
            .type = SYM_TARGET,
            .ident = "PINA",
            .loc_decl = EMPTY_LOC
        }
    }
};

struct AVRTarget target = atmega16;


void target_declare_symbols(struct SymTable* st)
{
    for (size_t i = 0; i < target.symbol_count; i++) {
        struct Symbol* sym = &target.symbols[i];
        if (!symtable_declare(st, SYM_TARGET, sym->ident, EMPTY_LOC)) {
            assert(0);
        }
    }
}
#ifndef AVR_TARGET_H
#define AVR_TARGET_H

#include "frontend/sema/symbols.h"


struct AVRTarget {
    const char* name;
    unsigned int zero_reg;
    unsigned int tmp_reg;
    unsigned int oprnd_reg_first;
    unsigned int oprnd_reg_num;
    unsigned int ret_reg;

    /// @brief Stackpointer size in bytes
    unsigned int sp_size_bytes;
    unsigned int ret_addr_size_bytes;

    size_t symbol_count;
    /// @brief Not owned: points to the symbol array of the preset this target
    /// was copied from. Must NOT be a flexible array member: `target` is
    /// assigned from a preset, and a struct assignment only copies
    /// `sizeof(struct AVRTarget)` - which excludes a flexible array member.
    const struct Symbol* symbols;
};

extern const struct AVRTarget atmega16;

extern struct AVRTarget target;

void target_declare_symbols(struct SymTable* st);

#endif
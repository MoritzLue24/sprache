#ifndef AVR_TARGET_H
#define AVR_TARGET_H

#include "sema/symbols.h"


struct AVRTarget {
    const char* name;
    unsigned int zero_reg;
    unsigned int tmp_reg;
    unsigned int oprnd_reg_first;
    unsigned int oprnd_reg_num;
    unsigned int ret_reg;

    size_t symbol_count;
    struct Symbol symbols[];
};

extern const struct AVRTarget atmega16;

extern struct AVRTarget target;

void target_declare_symbols(struct SymTable* st);

#endif
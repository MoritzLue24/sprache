#ifndef AVRGEN_INTERNAL_H
#define AVRGEN_INTERNAL_H

#include "backend/ir/irinstr.h"

#include <stdio.h>


struct RegsStr {
    char dest[16], src1[16], src2[16];
};

struct RegsStr get_regs_str(const struct IRInstr* instr);

void set_fctx(FILE* f);

FILE* ctx();

#endif
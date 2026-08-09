#include "backend/codegen/avrgen_internal.h"

#include "backend/target/avr_target.h"


static FILE* fctx = NULL;


struct RegsStr get_regs_str(const struct IRInstr* instr)
{
    struct RegsStr regs = {0};
    if (!instr->dest.none
        && instr->dest.type == OPRND_REG
        && instr->dest.reg.regalloc_done)
        snprintf(regs.dest, sizeof(regs.dest), "r%li", target.oprnd_reg_first + instr->dest.reg.preg_i);
    if (!instr->src1.none
        && instr->src1.type == OPRND_REG
        && instr->src1.reg.regalloc_done)
        snprintf(regs.src1, sizeof(regs.src1), "r%li", target.oprnd_reg_first + instr->src1.reg.preg_i);
    if (!instr->src2.none
        && instr->src2.type == OPRND_REG
        && instr->src2.reg.regalloc_done)
        snprintf(regs.src2, sizeof(regs.src2), "r%li", target.oprnd_reg_first + instr->src2.reg.preg_i);

    return regs;
}

void set_fctx(FILE* f)
{
    fctx = f;
}

FILE* ctx()
{
    return fctx;
}
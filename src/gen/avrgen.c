#include "gen/avrgen.h"
#include "gen/regalloc.h"
#include "target/avr_target.h"
#include <assert.h>
#include <string.h>


struct RegsStr { char dest[16], src1[16], src2[16]; };

static struct RegsStr get_regs_str(const struct IRInstr* instr)
{
    struct RegsStr regs = {0};
    if (!instr->dest.none && instr->dest.type == OPRND_PREG)
        snprintf(regs.dest, sizeof(regs.dest), "r%li", target.oprnd_reg_first + instr->dest.preg_i);
    if (!instr->src1.none && instr->src1.type == OPRND_PREG)
        snprintf(regs.src1, sizeof(regs.src1), "r%li", target.oprnd_reg_first + instr->src1.preg_i);
    if (!instr->src2.none && instr->src2.type == OPRND_PREG)
        snprintf(regs.src2, sizeof(regs.src2), "r%li", target.oprnd_reg_first + instr->src2.preg_i);

    return regs;
}

static void write_binop(FILE* out, const char* mnemonic, bool commutative, struct RegsStr regs_str)
{
    assert(regs_str.dest);
    assert(regs_str.src1);
    assert(regs_str.src2);

    if (strcmp(regs_str.dest, regs_str.src1) == 0)
        fprintf(out, "\t%s %s, %s\n", mnemonic, regs_str.dest, regs_str.src2);
    else if (strcmp(regs_str.dest, regs_str.src2) == 0) {
        if (commutative) {
            fprintf(out, "\t%s %s, %s\n", mnemonic, regs_str.dest, regs_str.src1);
        }
        else {
            fprintf(out, "\tmov r%i, %s\n", target.tmp_reg, regs_str.src1);
            fprintf(out, "\t%s r%i, %s\n", mnemonic, target.tmp_reg, regs_str.src2);
            fprintf(out, "\tmov %s, r%i\n", regs_str.dest, target.tmp_reg);
        }
    }
    else {
        fprintf(out, "\tmov %s, %s\n", regs_str.dest, regs_str.src1);
        fprintf(out, "\t%s %s, %s\n", mnemonic, regs_str.dest, regs_str.src2);
    }
}

bool gen_avr(const struct IRInstr* head, FILE* out)
{
    assert(head && out);

    fprintf(out, ".include \"m16def.inc\"\n");
    fprintf(out, ".cseg\n__init:\n");
    fprintf(out, "\tldi r16, high(RAMEND)\n\tout SPH, r16\n");
    fprintf(out, "\tldi r16, low(RAMEND)\n\tout SPL, r16\n");
    fprintf(out, "\tldi r16, 0xFF\n\tout DDRB, r16\n");
    fprintf(out, "\tcall main\n");
    fprintf(out, "\tout PORTB, r%i\n", target.ret_reg);
    fprintf(out, "__exit:\n\trjmp __exit\n");

    fprintf(out, "main:\n");

    for (const struct IRInstr* inst = head; inst != NULL; inst = inst->next) {
        struct RegsStr regs_str = get_regs_str(inst);

        switch (inst->op) {
            case IR_INIT_SF:
                assert(inst->src1.type == OPRND_IMM);
                fprintf(out, "\tpush r28\n\tpush r29\n");
                fprintf(out, "\tin r29, SPH\n\tin r28, SPL\n");
                fprintf(out, "\tsbiw r28, %i\n", inst->src1.imm);
                fprintf(out, "\tout SPH, r29\n\tout SPL, r28\n");
                fprintf(out, "\t; prologue end, stack size = %i\n", inst->src1.imm);
                break;

            case IR_FREE_SF:
                assert(inst->src1.type == OPRND_IMM);
                fprintf(out, "\t; epilogue start\n");
                fprintf(out, "_L_main_epilogue:\n");
                fprintf(out, "\tadiw r28, %i\n", inst->src1.imm);
                fprintf(out, "\tout SPH, r29\n\tout SPL, r28\n");
                fprintf(out, "\tpop r29\n\tpop r28\n");
                fprintf(out, "\tret\n");
                break;

            case IR_IMM:
                assert(regs_str.dest);
                assert(!inst->src1.none);
                assert(inst->src1.type == OPRND_IMM);

                fprintf(out, "\tldi %s, %i\n", regs_str.dest, inst->src1.imm);
                break;

            case IR_ADD:
                write_binop(out, "add", true, regs_str);
                break;

            case IR_SUB:
                write_binop(out, "sub", false, regs_str);
                break;

            case IR_MUL:
                write_binop(out, "mul", true, regs_str);
                // `mul` result is saved to R0 & R1 (high byte)
                // R1 discarded for now
                fprintf(out, "\tmov %s, R0\n", regs_str.dest);
                break;

            case IR_RETURN:
                assert(regs_str.src1);
                fprintf(out, "\tmov r%i, %s\n", target.ret_reg, regs_str.src1);
                fprintf(out, "\tjmp _L_main_epilogue\n");
                break;

            case IR_LOAD_LOCAL:
                assert(regs_str.dest);
                assert(!inst->src1.none);
                assert(inst->src1.type == OPRND_VAR);

                fprintf(out, "\tldd %s, Y+%i\n", regs_str.dest, inst->src1.sf_offset);
                break;

            case IR_STORE_LOCAL:
                assert(regs_str.src1);
                assert(!inst->dest.none);
                assert(inst->dest.type == OPRND_VAR);

                fprintf(out, "\tstd Y+%i, %s\n", inst->dest.sf_offset, regs_str.src1);
                break;

            default:
                assert(0);
        }
    }
    return true;
}
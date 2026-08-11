#include "backend/codegen/avrgen.h"
#include "backend/codegen/avrgen_internal.h"

#include <assert.h>
#include <string.h>

#include "backend/codegen/regalloc.h"
#include "backend/target/avr_target.h"


// forward decl
static void write_func(const struct IRFunc* fn);
static void write_instr(const struct IRInstr* instr);
static void write_alloc_sf(const struct IRInstr* instr);
static void write_drop_sf(const struct IRInstr* instr);
static void write_push_arg(const struct IRInstr* instr);
static void write_call(const struct IRInstr* instr);
static void write_pop_arg(const struct IRInstr* instr);
static void write_imm(const struct IRInstr* instr);
static void write_unary_op(const struct IRInstr* instr, const char* mnemonic);
static void write_binop(const struct IRInstr* instr, const char* mnemonic, bool commutative);
static void write_return(const struct IRInstr* instr);
static void write_load_local(const struct IRInstr* instr);
static void write_store_local(const struct IRInstr* instr);


void gen_avr(const struct IRFunc* head, FILE* out)
{
    assert(head && out);
    set_fctx(out);

    fprintf(ctx(), ".include \"m16def.inc\"\n");
    fprintf(ctx(), ".cseg\n__init:\n");
    fprintf(ctx(), "\tldi r16, high(RAMEND)\n\tout SPH, r16\n");
    fprintf(ctx(), "\tldi r16, low(RAMEND)\n\tout SPL, r16\n");
    fprintf(ctx(), "\tldi r16, 0xFF\n\tout DDRB, r16\n");
    fprintf(ctx(), "\tcall main\n");
    fprintf(ctx(), "\tout PORTB, r%i\n", target.ret_reg);
    fprintf(ctx(), "__exit:\n\trjmp __exit\n");

    for (const struct IRFunc* cur = head; cur != NULL; cur = cur->next) {
        write_func(cur);
    }
}

static void write_func(const struct IRFunc* fn)
{
    fprintf(ctx(), "%s:\n", fn->ident);
    for (const struct IRInstr* cur = fn->instrs; cur != NULL; cur = cur->next) {
        write_instr(cur);
    }
}

static void write_instr(const struct IRInstr* instr)
{
    switch (instr->op) {
        case IR_ALLOC_SF:
            write_alloc_sf(instr);
            break;
        case IR_DROP_SF:
            write_drop_sf(instr);
            break;
        case IR_PUSH_ARG:
            write_push_arg(instr);
            break;
        case IR_CALL:
            write_call(instr);
            break;
        case IR_POP_ARG:
            write_pop_arg(instr);
            break;
        case IR_IMM:
            write_imm(instr);
            break;
        case IR_NEG:
            write_unary_op(instr, "neg");
            break;
        case IR_ADD:
            write_binop(instr, "add", true);
            break;
        case IR_SUB:
            write_binop(instr, "sub", false);
            break;
        case IR_MUL:
            write_binop(instr, "mul", true);
            break;
        case IR_RETURN:
            write_return(instr);
            break;
        case IR_LOAD_LOCAL:
            write_load_local(instr);
            break;
        case IR_STORE_LOCAL:
            write_store_local(instr);
            break;
        default:
            assert(0);
    }
}

static void write_alloc_sf(const struct IRInstr* instr)
{
    assert(instr->op == IR_ALLOC_SF);
    assert(instr->src1.type == OPRND_IMM);

    fprintf(ctx(), "\tpush r28\n\tpush r29\n");
    fprintf(ctx(), "\tin r29, SPH\n\tin r28, SPL\n");
    fprintf(ctx(), "\tsbiw r28, %i\n", instr->src1.imm.value);
    fprintf(ctx(), "\tout SPH, r29\n\tout SPL, r28\n");
    fprintf(ctx(), "\t; prologue end, stack size = %i\n", instr->src1.imm.value);
}

static void write_drop_sf(const struct IRInstr* instr)
{
    assert(instr->op == IR_DROP_SF);
    assert(instr->src1.type == OPRND_IMM);
    assert(instr->src2.type == OPRND_FUNC);

    fprintf(ctx(), "\t; epilogue start\n");
    fprintf(ctx(), "_L_%s_epilogue:\n", instr->src2.func.ident);
    fprintf(ctx(), "\tadiw r28, %i\n", instr->src1.imm.value);
    fprintf(ctx(), "\tout SPH, r29\n\tout SPL, r28\n");
    fprintf(ctx(), "\tpop r29\n\tpop r28\n");
    fprintf(ctx(), "\tret\n");
}

static void write_push_arg(const struct IRInstr* instr)
{
    assert(instr->op == IR_PUSH_ARG);
    assert(instr->src1.type == OPRND_REG && instr->src1.reg.regalloc_done);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tpush %s\n", regs.src1);
}

static void write_call(const struct IRInstr* instr)
{
    assert(instr->op == IR_CALL);
    assert(instr->dest.type == OPRND_REG && instr->dest.reg.regalloc_done);
    assert(instr->src1.type == OPRND_FUNC);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tcall %s\n", instr->src1.func.ident);
    fprintf(ctx(), "\tmov %s, r%i\n", regs.dest, target.ret_reg);
}

static void write_pop_arg(const struct IRInstr* instr)
{
    assert(instr->op == IR_POP_ARG);
    fprintf(ctx(), "\tpop r%i\n", target.tmp_reg);
}

static void write_imm(const struct IRInstr* instr)
{
    assert(instr->op == IR_IMM);
    assert(instr->dest.type == OPRND_REG && instr->dest.reg.regalloc_done);
    assert(instr->src1.type == OPRND_IMM);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tldi %s, %i\n", regs.dest, instr->src1.imm.value);
}

static void write_unary_op(const struct IRInstr* instr, const char* mnemonic)
{
    assert(instr->dest.type == OPRND_REG && instr->dest.reg.regalloc_done);
    assert(instr->src1.type == OPRND_REG && instr->src1.reg.regalloc_done);

    struct RegsStr regs = get_regs_str(instr);
    if (strcmp(regs.dest, regs.src1) == 0) {
        fprintf(ctx(), "\t%s %s\n", mnemonic, regs.src1);
    }
    else {
        fprintf(ctx(), "\tmov r%i, %s\n", target.tmp_reg, regs.src1);
        fprintf(ctx(), "\t%s r%i\n", mnemonic, target.tmp_reg);
        fprintf(ctx(), "\tmov %s, r%i\n", regs.dest, target.tmp_reg);
    }
}

static void write_binop(const struct IRInstr* instr, const char* mnemonic, bool commutative)
{
    assert(instr->dest.type == OPRND_REG && instr->dest.reg.regalloc_done);
    assert(instr->src1.type == OPRND_REG && instr->src1.reg.regalloc_done);
    assert(instr->src2.type == OPRND_REG && instr->src2.reg.regalloc_done);

    struct RegsStr regs = get_regs_str(instr);

    if (strcmp(mnemonic, "mul") == 0) {
        fprintf(ctx(), "\tmul %s, %s\n", regs.src1, regs.src2);
        fprintf(ctx(), "\tmov %s, r0\n", regs.dest);
        return;
    }

    if (strcmp(regs.dest, regs.src1) == 0)
        fprintf(ctx(), "\t%s %s, %s\n", mnemonic, regs.dest, regs.src2);
    else if (strcmp(regs.dest, regs.src2) == 0) {
        if (commutative) {
            fprintf(ctx(), "\t%s %s, %s\n", mnemonic, regs.dest, regs.src1);
        }
        else {
            fprintf(ctx(), "\tmov r%i, %s\n", target.tmp_reg, regs.src1);
            fprintf(ctx(), "\t%s r%i, %s\n", mnemonic, target.tmp_reg, regs.src2);
            fprintf(ctx(), "\tmov %s, r%i\n", regs.dest, target.tmp_reg);
        }
    }
    else {
        fprintf(ctx(), "\tmov %s, %s\n", regs.dest, regs.src1);
        fprintf(ctx(), "\t%s %s, %s\n", mnemonic, regs.dest, regs.src2);
    }
}

static void write_return(const struct IRInstr* instr)
{
    assert(instr->src1.type == OPRND_REG && instr->src1.reg.regalloc_done);
    assert(instr->src2.type == OPRND_FUNC);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tmov r%i, %s\n", target.ret_reg, regs.src1);
    fprintf(ctx(), "\tjmp _L_%s_epilogue\n", instr->src2.func.ident);
}

static void write_load_local(const struct IRInstr* instr)
{
    assert(instr->dest.type == OPRND_REG && instr->dest.reg.regalloc_done);
    assert(instr->src1.type == OPRND_VAR && !instr->src1.var.sf_entry->needs_resolving);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tldd %s, Y+%i\n", regs.dest, instr->src1.var.sf_entry->offset);
}

static void write_store_local(const struct IRInstr* instr)
{
    assert(instr->dest.type == OPRND_VAR && !instr->dest.var.sf_entry->needs_resolving);
    assert(instr->src1.type == OPRND_REG && instr->src1.reg.regalloc_done);

    struct RegsStr regs = get_regs_str(instr);
    fprintf(ctx(), "\tstd Y+%i, %s\n", instr->dest.var.sf_entry->offset, regs.src1);
}
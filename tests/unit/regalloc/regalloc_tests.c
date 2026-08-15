#include "tst.h"

#include "backend/ir/irinstr.h"
#include "backend/codegen/regalloc.h"


static void regalloc_when_two_not_conflicting_reuses_preg()
{
    /*
    vr0 <- 9
    vr1 <- 9
    */
    struct IRInstr* a = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 9
        },
        EMPTY_OPRND
    );
    struct IRInstr* b = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 1
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 9
        },
        EMPTY_OPRND
    );
    a->next = b;

    regalloc(a);
    TST_ASSERT_EQ(true, a->dest.reg.regalloc_done);
    TST_ASSERT_EQ(true, b->dest.reg.regalloc_done);
    TST_ASSERT_EQ(0, a->dest.reg.preg_i);
    TST_ASSERT_EQ(0, b->dest.reg.preg_i);

    free_irlist(a);
}

static void regalloc_when_overlapping_lifespans_uses_different_pregs()
{
    /*
    vr0 <- 9
    vr1 <- 13
    vr2 <- vr0 + vr1
    */
    struct IRInstr* a = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 9
        },
        EMPTY_OPRND
    );
    struct IRInstr* b = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 1
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 13
        },
        EMPTY_OPRND
    );
    struct IRInstr* c = new_instr(
        IR_ADD,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 2
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 1
        }
    );
    a->next = b;
    b->next = c;

    regalloc(a);
    // vr0 and vr1 interfere -> distinct pregs
    TST_ASSERT_EQ(0, a->dest.reg.preg_i);
    TST_ASSERT_EQ(1, b->dest.reg.preg_i);
    // vr2 interferes with neither -> reuses first open preg
    TST_ASSERT_EQ(0, c->dest.reg.preg_i);
    // operands referencing vr0/vr1 got resolved to the same pregs as their defs
    TST_ASSERT_EQ(0, c->src1.reg.preg_i);
    TST_ASSERT_EQ(1, c->src2.reg.preg_i);

    free_irlist(a);
}

static void regalloc_overwrite_last_use_reuses_preg()
{
    /*
    vr0 <- 9
    vr1 <- vr0 + 13
    vr0 <- 7
    */
    struct IRInstr* a = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 9
        },
        EMPTY_OPRND
    );
    struct IRInstr* b = new_instr(
        IR_ADD,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 1
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 13
        }
    );
    struct IRInstr* c = new_instr(
        IR_IMM,
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_IMM,
            .imm.value = 7
        },
        EMPTY_OPRND
    );
    a->next = b;
    b->next = c;

    regalloc(a);
    // vr0 and vr1 don't interfere -> same preg
    TST_ASSERT_EQ(0, a->dest.reg.preg_i);
    TST_ASSERT_EQ(0, b->dest.reg.preg_i);
    TST_ASSERT_EQ(0, b->src1.reg.preg_i);
    TST_ASSERT_EQ(0, c->dest.reg.preg_i);

    free_irlist(a);
}

static void regalloc_more_vregs_than_pregs_when_not_conflicting()
{
    /*
    vr0 <- 1
    vr1 <- 2
    vr2 <- 3
    vr3 <- 4
    vr4 <- 5
    (target has only 4 physical registers, but none of these vregs ever overlap)
    */
    struct IRInstr* head = NULL;
    struct IRInstr* tail = NULL;
    for (size_t i = 0; i < 5; i++) {
        struct IRInstr* instr = new_instr(
            IR_IMM,
            (struct IROperand){
                .none = false,
                .type = OPRND_REG,
                .reg.regalloc_done = false,
                .reg.vreg_i = i
            },
            (struct IROperand){
                .none = false,
                .type = OPRND_IMM,
                .imm.value = (int)(i + 1)
            },
            EMPTY_OPRND
        );
        if (head == NULL) head = instr;
        else tail->next = instr;
        tail = instr;
    }

    regalloc(head);
    for (struct IRInstr* instr = head; instr != NULL; instr = instr->next) {
        TST_ASSERT_EQ(true, instr->dest.reg.regalloc_done);
        TST_ASSERT_EQ(0, instr->dest.reg.preg_i);
    }

    free_irlist(head);
}

static void regalloc_when_four_vregs_mutually_interfere_uses_all_pregs()
{
    /*
    vr0 <- 1
    vr1 <- 2
    vr2 <- 3
    vr3 <- 4
    vr4 <- vr0 + vr1
    vr5 <- vr2 + vr3
    (vr0..vr3 are all pairwise live at the same time, using exactly the
    4 physical registers of the target)
    */
    struct IRInstr* a = new_instr(
        IR_IMM,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 0 },
        (struct IROperand){ .none = false, .type = OPRND_IMM, .imm.value = 1 },
        EMPTY_OPRND
    );
    struct IRInstr* b = new_instr(
        IR_IMM,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 1 },
        (struct IROperand){ .none = false, .type = OPRND_IMM, .imm.value = 2 },
        EMPTY_OPRND
    );
    struct IRInstr* c = new_instr(
        IR_IMM,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 2 },
        (struct IROperand){ .none = false, .type = OPRND_IMM, .imm.value = 3 },
        EMPTY_OPRND
    );
    struct IRInstr* d = new_instr(
        IR_IMM,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 3 },
        (struct IROperand){ .none = false, .type = OPRND_IMM, .imm.value = 4 },
        EMPTY_OPRND
    );
    struct IRInstr* e = new_instr(
        IR_ADD,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 4 },
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 0 },
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 1 }
    );
    struct IRInstr* f = new_instr(
        IR_ADD,
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 5 },
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 2 },
        (struct IROperand){ .none = false, .type = OPRND_REG, .reg.regalloc_done = false, .reg.vreg_i = 3 }
    );
    a->next = b;
    b->next = c;
    c->next = d;
    d->next = e;
    e->next = f;

    regalloc(a);
    TST_ASSERT_EQ(0, a->dest.reg.preg_i);
    TST_ASSERT_EQ(1, b->dest.reg.preg_i);
    TST_ASSERT_EQ(2, c->dest.reg.preg_i);
    TST_ASSERT_EQ(3, d->dest.reg.preg_i);

    free_irlist(a);
}

int main()
{
    TST_RUN(regalloc_when_two_not_conflicting_reuses_preg);
    TST_RUN(regalloc_when_overlapping_lifespans_uses_different_pregs);
    TST_RUN(regalloc_overwrite_last_use_reuses_preg);
    TST_RUN(regalloc_more_vregs_than_pregs_when_not_conflicting);
    TST_RUN(regalloc_when_four_vregs_mutually_interfere_uses_all_pregs);
    TST_SUMMARY();
}

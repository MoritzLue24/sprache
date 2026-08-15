#include "tst.h"

#include "backend/ir/irinstr.h"
#include "backend/codegen/regalloc.h"


static void interf_graph_should_be_empty_when_no_instrs()
{
    struct InterfGraph g = create_interf_graph(NULL);
    TST_ASSERT_EQ(0, g.n);
}

static void interf_graph_when_two_not_conflicting()
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

    struct InterfGraph g = create_interf_graph(a);
    TST_ASSERT_EQ(2, g.n);
    for (size_t i = 0; i < g.n * 2; i++) {
        TST_ASSERT_EQ(false, g.adj[i]);
    } 

    free_interf_graph(g);
    free_irlist(a);
}

static void interf_graph_when_two_not_conflicting_same_instr()
{
    /*
    vr0 <- 9
    vr1 <- 9 + vr0
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
            .type = OPRND_IMM,
            .imm.value = 9
        },
        (struct IROperand){
            .none = false,
            .type = OPRND_REG,
            .reg.regalloc_done = false,
            .reg.vreg_i = 0
        }
    );
    a->next = b;

    struct InterfGraph g = create_interf_graph(a);
    TST_ASSERT_EQ(2, g.n);
    for (size_t i = 0; i < g.n * 2; i++) {
        TST_ASSERT_EQ(false, g.adj[i]);
    } 

    free_interf_graph(g);
    free_irlist(a);
}

static void interf_graph_when_overlapping_lifespans()
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

    struct InterfGraph g = create_interf_graph(a);
    TST_ASSERT_EQ(3, g.n);
    TST_ASSERT_EQ(true, g.adj[0*g.n + 1]);
    TST_ASSERT_EQ(true, g.adj[1*g.n + 0]);
    TST_ASSERT_EQ(false, g.adj[2*g.n + 0]);
    TST_ASSERT_EQ(false, g.adj[0*g.n + 2]);
    TST_ASSERT_EQ(false, g.adj[2*g.n + 1]);
    TST_ASSERT_EQ(false, g.adj[1*g.n + 2]);

    free_interf_graph(g);
    free_irlist(a);
}

static void interf_graph_overwrite_last_use()
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

    struct InterfGraph g = create_interf_graph(a);
    TST_ASSERT_EQ(false, g.adj[0*g.n + 1]);
    TST_ASSERT_EQ(false, g.adj[1*g.n + 0]);

    free_interf_graph(g);
    free_irlist(a);
}

int main()
{
    TST_RUN(interf_graph_should_be_empty_when_no_instrs);
    TST_RUN(interf_graph_when_two_not_conflicting);
    TST_RUN(interf_graph_when_two_not_conflicting_same_instr);
    TST_RUN(interf_graph_when_overlapping_lifespans);
    TST_RUN(interf_graph_overwrite_last_use);
    TST_SUMMARY();
}
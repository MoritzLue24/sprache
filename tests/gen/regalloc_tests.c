#include "tst.h"
#include "gen/regalloc.h"


static struct IROperand init_vreg(size_t vreg_i) {
    struct IROperand o;
    o.none = false;
    o.type = OPRND_VREG;
    o.vreg_i = vreg_i;
    return o;
}
 
static struct IROperand init_imm(int val) {
    struct IROperand o;
    o.none = false;
    o.type = OPRND_IMM;
    o.imm = val;
    return o;
}
 
static struct IROperand empty_oprn() {
    struct IROperand o;
    o.none = true;
    o.type = OPRND_IMM;
    o.imm = 0;
    return o;
}

static void apply_preg_allocs(struct IRInstr* head, int* preg_allocs)
{
    if (!head->dest.none && head->dest.type == OPRND_VREG) {
        head->dest.vreg_i = preg_allocs[head->dest.vreg_i];
    }
    if (!head->src1.none && head->src1.type == OPRND_VREG) {
        head->src1.vreg_i = preg_allocs[head->src1.vreg_i];
    }
    if (!head->src2.none && head->src2.type == OPRND_VREG) {
        head->src2.vreg_i = preg_allocs[head->src2.vreg_i];
    }
    if (head->next != NULL) apply_preg_allocs(head->next, preg_allocs);
}


void a()
{
    // V0 <- 5 
    // V1 <- 10
    // V2 <- V0, V1
    // exit V2
    struct IRInstr ir_list[4];
    ir_list[0] = (struct IRInstr){
        .op = IR_IMM,
        .dest = init_vreg(0),
        .src1 = init_imm(5),
        .src2 = empty_oprn(),
        .next = &ir_list[1],
    };
    ir_list[1] = (struct IRInstr){
        .op = IR_IMM,
        .dest = init_vreg(1),
        .src1 = init_imm(10),
        .src2 = empty_oprn(),
        .next = &ir_list[2],
    };
    ir_list[2] = (struct IRInstr){
        .op = IR_ADD,
        .dest = init_vreg(2),
        .src1 = init_vreg(0),
        .src2 = init_vreg(1),
        .next = &ir_list[3],
    };
    ir_list[3] = (struct IRInstr){
        .op = IR_ADD,
        .dest = init_vreg(8),
        .src1 = init_vreg(1),
        .src2 = init_vreg(3),
        .next = &ir_list[4],
    };
    ir_list[4] = (struct IRInstr){
        .op = IR_EXIT,
        .dest = empty_oprn(),
        .src1 = init_vreg(2),
        .src2 = empty_oprn(),
        .next = NULL,
    };
    printf("\n\n");
    print_irlist(&ir_list[0]);
    printf("\n\n");

    struct InterfGraph g = create_interf_graph(&ir_list[0]);
    print_adj_matrix(g);
}

int main()
{
    TST_RUN(a);
    TST_SUMMARY();
    return 0;
}
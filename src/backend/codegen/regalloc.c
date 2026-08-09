#include "backend/codegen/regalloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "utils/xalloc.h"
#include "backend/target/avr_target.h"


static bool is_vreg(struct IROperand op)
{
    return !op.none && op.type == OPRND_REG && !op.reg.regalloc_done;
}

/// Assumes VReg.i starts at 0
static size_t count_vregs(const struct IRInstr* head)
{
    size_t max = 0;
    bool any = false;
    for (; head != NULL; head = head->next) {
        if (is_vreg(head->dest) && head->dest.reg.vreg_i > max) {
                max = head->dest.reg.vreg_i;
                any = true;
            }
        if (is_vreg(head->src1) && head->src1.reg.vreg_i > max) {
                max = head->src1.reg.vreg_i;
                any = true;
            }
        if (is_vreg(head->src2) && head->src2.reg.vreg_i > max) {
                max = head->src2.reg.vreg_i;
                any = true;
            }
    }
    return any ? max + 1 : 0;
}

struct InterfGraph create_interf_graph(const struct IRInstr* head)
{
    size_t n = count_vregs(head);
    // instruction index where the vreg was defined first
    // (def_inx[i] : instruction index of register with i=i)
    int* def_idx = xcalloc(n, sizeof(int));
    // instruction index where the vreg was used last
    // mapping like in def_idx
    int* last_use_idx = xcalloc(n, sizeof(int));

    for (size_t i = 0; i < n; i++) {
        def_idx[i] = -1;
        last_use_idx[i] = -1;
    }

    unsigned int inst_i = 0;
    for (const struct IRInstr* inst = head; inst != NULL; inst = inst->next, inst_i++) {
        if (is_vreg(inst->dest)) {
            def_idx[inst->dest.reg.vreg_i] = inst_i;
        }
        if (is_vreg(inst->dest)) {
            last_use_idx[inst->src1.reg.vreg_i] = inst_i;
        }
        if (is_vreg(inst->dest)) {
            last_use_idx[inst->src2.reg.vreg_i] = inst_i;
        }
    }

    // degree of each vreg-node
    // unsigned int* degree = calloc(n, sizeof(unsigned int));
    // adj-matrix: adj[i*n + j]: "i is adjacent to j"
    // (symmetric)
    bool* adj = calloc(n * n, sizeof(bool));

    // i, j : indices of vregs
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
                // i lives inside j
            bool overlaps = (def_idx[i] < last_use_idx[j] && def_idx[i] > def_idx[j])
                // j lives inside i
                || (def_idx[j] < last_use_idx[i] && def_idx[j] > def_idx[i]);
            if (overlaps) {
                adj[i * n + j] = adj[j * n + i] = 1;
                // degree[i]++; degree[j]++;
            }
        }
    }
    xfree((void**)&def_idx);
    xfree((void**)&last_use_idx);
    return (struct InterfGraph){ .n = n, .adj = adj };
}

void print_adj_matrix(struct InterfGraph g)
{
    printf("  |");
    for (size_t j = 0; j < g.n; j++) {
        if (j != g.n - 1) printf(" %zu |", j);
        else printf(" %zu\n", j);
    }
    for (size_t i = 0; i < g.n; i++) {
        printf("%zu |", i);
        for (size_t j = 0; j < g.n; j++) {
            if (j != g.n - 1) printf(" %i |", g.adj[i * g.n + j]);
            else printf(" %i\n", g.adj[i * g.n + j]);
        }
    }
}

void free_interf_graph(struct InterfGraph g)
{
    xfree((void**)&g.adj);
    // xfree((void**)&g.degree);
}

void regalloc(struct IRInstr* head)
{
    struct InterfGraph g = create_interf_graph(head);

    // an array of physical registers indices, of length g.n
    // where preg_allocs[i] = x <-> "vreg i allocated to preg oprnd_reg_first + x".
    int* preg_allocs = xcalloc(g.n, sizeof(int));
    for (size_t i = 0; i < g.n; i++) {
        preg_allocs[i] = -1;
    }

    for (size_t i = 0; i < g.n; i++) {
        // adj_uses_pregs[i] = true <-> "preg i is used by neighbor"
        bool* adj_uses_preg = calloc(target.oprnd_reg_num, sizeof(bool));

        // Loop through neighbors
        // set 'adj_uses_pregs'
        for (size_t j = 0; j < g.n; j++) {
            if (g.adj[i * g.n + j] == 0) continue;

            int used_preg = preg_allocs[j];
            if (used_preg != -1) {
                adj_uses_preg[used_preg] = true;
            }
        }

        // open_preg = x <-> "preg FIRST_PREG + x is open and ready"
        int open_preg = -1;
        for (size_t j = 0; j < target.oprnd_reg_num; j++) {
            if (adj_uses_preg[j]) continue;
            open_preg = j;
            break;
        }
        xfree((void**)&adj_uses_preg);

        // set preg
        assert(open_preg != -1);
        preg_allocs[i] = open_preg;
    }

    // Set every IROperand.preg_i accordingly
    free_interf_graph(g);
    for (struct IRInstr* instr = head; instr != NULL; instr = instr->next) {
        if (is_vreg(instr->dest)) {
            instr->dest.reg.regalloc_done = true;
            instr->dest.reg.preg_i = preg_allocs[instr->dest.reg.vreg_i];
        }
        if (is_vreg(instr->src1)) {
            instr->src1.reg.regalloc_done = true;
            instr->src1.reg.preg_i = preg_allocs[instr->src1.reg.vreg_i];
        }
        if (is_vreg(instr->src2)) {
            instr->src2.reg.regalloc_done = true;
            instr->src2.reg.preg_i = preg_allocs[instr->src2.reg.vreg_i];
        }
    }
    xfree((void**)&preg_allocs);
}

#ifndef REGALLOC_H
#define REGALLOC_H

#include "gen/irinstr.h"


/// @brief Interference graph
struct InterfGraph {
    // num of total vregs
    size_t n;
    // adj-matrix: adj[i*n + j]: "i is adjacent to j" <-> "i lives in the same scope as j"
    // (symmetric)
    bool* adj;
    // degree of each vreg-node, where degree[i] is the number of other vregs, the vreg 'i' lives with
    // unsigned int* degree;
};

/// @brief Creates a graph where: Node=vreg, Edge="lives in same 'scope'"
/// @note needs freeing: `free_graph(g)`
struct InterfGraph create_interf_graph(const struct IRInstr* head);

/// @brief prints the adjacent matrix to the graph. 
/// The matrix should be symmetrical, but NOT reflexive
void print_adj_matrix(struct InterfGraph g);

void free_interf_graph(struct InterfGraph g);

/// @brief Allocates a physical register (in 0 to oprnd_reg_num - 1) to
/// each virtual register to minimize the number multiple physical registers used at once.
///
/// @note see https://www.youtube.com/watch?v=K3mi2m7ccDQ
void regalloc(struct IRInstr* head);

#endif
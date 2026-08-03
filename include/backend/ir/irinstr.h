#ifndef IRINSTR_H
#define IRINSTR_H

#include <stdlib.h>
#include <stdbool.h>


enum IROp {
    /// @brief initialize stackframe, src1: imm, number of bytes to reserve in the stackframe
    IR_INIT_SF,
    /// @brief free stackfarme, src1 same as init sf
    IR_FREE_SF,

    /// @brief dest: imm
    IR_IMM,
    /// @brief dest = src1 + src2
    IR_ADD,
    /// @brief dest = src1 - src2
    IR_SUB,
    /// @brief dest = src1 * src2
    IR_MUL,

    /// @brief return src1
    IR_RETURN,
    /// @brief load local variable from stackframe
    IR_LOAD_LOCAL,
    /// @brief store local variable from stackframe
    IR_STORE_LOCAL,
};

enum OperandType {
    OPRND_PREG,
    OPRND_VREG,
    OPRND_IMM,
    OPRND_VAR
};

struct IROperand {
    bool none;
    enum OperandType type;
    union {
        size_t preg_i;
        size_t vreg_i;
        int imm;
        unsigned int sf_offset;
    } ;
};

#define EMPTY_OPRND (struct IROperand) { .none = true }

struct IRInstr {
    enum IROp op;
    struct IROperand dest;
    struct IROperand src1;
    struct IROperand src2;
    struct IRInstr* next;
};


const char* irop_str(enum IROp op);

/// @note needs freeing 
char* oprnd_str(struct IROperand oprnd);

void print_irlist(struct IRInstr* head);

void free_irlist(struct IRInstr* head);

#endif
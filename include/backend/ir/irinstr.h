#ifndef IRINSTR_H
#define IRINSTR_H

#include <stdlib.h>
#include <stdbool.h>

#include "backend/ir/stack_frame.h"


enum IROp {
    /// @brief initialize stackframe, src1: imm, number of bytes to reserve in the stackframe
    IR_ALLOC_SF,
    /// @brief src1: stacksize, src2: func
    IR_DROP_SF,

    /// @brief src1: vreg
    IR_PUSH_ARG, 
    /// @brief dest: return val (vreg), src1: function ident (func), src2: arg count (imm)
    IR_CALL,
    IR_POP_ARG,

    /// @brief dest: imm
    IR_IMM,
    /// @brief -src1: reg
    IR_NEG,
    /// @brief ~src1: reg
    IR_COM,
    /// @brief dest = src1 | src2
    IR_OR,
    /// @brief dest = src1 ^ src2
    IR_XOR,
    /// @brief dest = src1 & src2
    IR_AND,
    /// @brief dest = src1 + src2
    IR_ADD,
    /// @brief dest = src1 - src2
    IR_SUB,
    /// @brief dest = src1 * src2
    IR_MUL,

    /// @brief return src1, src2: func
    IR_RETURN,
    /// @brief load local variable from stackframe
    IR_LOAD_LOCAL,
    /// @brief store local variable from stackframe
    IR_STORE_LOCAL,
};

enum OperandType {
    OPRND_REG,
    OPRND_IMM,
    OPRND_VAR,
    OPRND_FUNC
};

struct IROperand {
    bool none;
    enum OperandType type;
    union {
        struct {
            /// @brief True if we already reg allocated for this instr.
            /// False if we only have vregs
            bool regalloc_done;
            size_t preg_i;
            size_t vreg_i;
        } reg;

        struct {
            int value;
        } imm;

        struct {
            struct SFEntry* sf_entry;
            /// @brief true if the real sf_offset is not yet set.
            /// for argument variables, sf_offset gets set by calculating
            /// the real offset, relative to the actual SF pointer
            //bool conv_done;
            //unsigned int sf_offset;
            /// @brief Offset, relative to BP + ret_addr_size
            //unsigned int rel_arg_offset;
        } var;

        struct {
            const char* ident;
        } func;
    };
};

#define EMPTY_OPRND (struct IROperand) { .none = true }

struct IRInstr {
    enum IROp op;
    struct IROperand dest;
    struct IROperand src1;
    struct IROperand src2;
    struct IRInstr* next;
};

struct IRFunc {
    char* ident;
    // size_t param_count;
    struct StackFrame sf;
    struct IRInstr* instrs;
    struct IRFunc* next;
};

/// @note needs freeing 
char* oprnd_str(struct IROperand oprnd);

void print_irfunc(const struct IRFunc* funclist);

void free_irfunc(struct IRFunc* func);

#endif
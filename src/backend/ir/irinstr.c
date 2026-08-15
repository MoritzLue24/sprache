#include "backend/ir/irinstr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils/xalloc.h"


struct IRInstr* new_instr(enum IROp op, struct IROperand dest, struct IROperand src1, struct IROperand src2)
{
    struct IRInstr* instr = xmalloc(sizeof(struct IRInstr));
    instr->op = op;
    instr->dest = dest;
    instr->src1 = src1;
    instr->src2 = src2;
    instr->next = NULL;
    return instr;
}

char* oprnd_str(struct IROperand oprnd)
{
    if (oprnd.none) {
        char* val = xmalloc(strlen("<none>") + 1);
        snprintf(val, strlen("<none>") + 1, "<none>");
        return val;
    }
    switch (oprnd.type) {
        case OPRND_VAR: {
            char* str;
            if (!oprnd.var.sf_entry->needs_resolving) {
                int len = snprintf(NULL, 0, "local[%i]", oprnd.var.sf_entry->offset);
                str = xmalloc(len + 1);
                snprintf(str, len + 1, "local[%i]", oprnd.var.sf_entry->offset); // \0 automatically set
            }
            else {
                int len = snprintf(NULL, 0, "arg[%i]", oprnd.var.sf_entry->rel_arg_offset);
                str = xmalloc(len + 1);
                snprintf(str, len + 1, "arg[%i]", oprnd.var.sf_entry->rel_arg_offset);
            }
            return str;
        }

        case OPRND_IMM: {
            int len = snprintf(NULL, 0, "[%i]", oprnd.imm.value);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "[%i]", oprnd.imm.value); // \0 automatically set
            return str;
        }

        case OPRND_REG: {
            char* str;
            if (oprnd.reg.regalloc_done) {
                int len = snprintf(NULL, 0, "r%zu", oprnd.reg.preg_i);
                str = xmalloc(len + 1);
                snprintf(str, len + 1, "r%zu", oprnd.reg.preg_i);
            }
            else {
                int len = snprintf(NULL, 0, "vr%zu", oprnd.reg.vreg_i);
                str = xmalloc(len + 1);
                snprintf(str, len + 1, "vr%zu", oprnd.reg.vreg_i);
            }
            return str;
        }

        case OPRND_FUNC: {
            int len = snprintf(NULL, 0, "'%s'", oprnd.func.ident);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "'%s'", oprnd.func.ident);
            return str;
        }

        default:
            assert(0);
    }
}

void print_irlist(const struct IRInstr* head, int depth)
{
    assert(head);

    char* dest = oprnd_str(head->dest);
    char* src1 = oprnd_str(head->src1);
    char* src2 = oprnd_str(head->src2);

    printf("%*s", depth * 4, "");
    switch (head->op) {
        case IR_ALLOC_SF:
            printf("ALLOC_SF, %s\n", src1);
            break;
        case IR_DROP_SF:
            printf("DROP_SF, %s\n", src1);
            break;
        case IR_PUSH_ARG:
            printf("PUSH_ARG, %s\n", src1);
            break;
        case IR_CALL:
            printf("%s <- CALL %s\n", dest, src1);
            break;
        case IR_POP_ARG:
            printf("POP_ARG\n");
            break;
        case IR_IMM:
            printf("%s <- %s\n", dest, src1);
            break;
        case IR_NEG:
            printf("%s <- NEG %s\n", dest, src1);
            break;
        case IR_COM:
            printf("%s <- COM %s\n", dest, src1);
            break;
        case IR_OR:
            printf("%s <- %s | %s\n", dest, src1, src2);
            break;
        case IR_XOR:
            printf("%s <- %s ^ %s\n", dest, src1, src2);
            break;
        case IR_AND:
            printf("%s <- %s & %s\n", dest, src1, src2);
            break;
        case IR_ADD:
            printf("%s <- %s + %s\n", dest, src1, src2);
            break;
        case IR_SUB:
            printf("%s <- %s - %s\n", dest, src1, src2);
            break;
        case IR_MUL:
            printf("%s <- %s * %s\n", dest, src1, src2);
            break;
        case IR_RETURN:
            printf("return %s\n", src1);
            break;
        case IR_LOAD_LOCAL:
            printf("%s <- %s\n", dest, src1);
            break;
        case IR_STORE_LOCAL:
            printf("%s <- %s\n", dest, src1);
            break;
        default:
            assert(0);
    }

    xfree((void**)&dest);
    xfree((void**)&src1);
    xfree((void**)&src2);

    if (head->next != NULL)
        print_irlist(head->next, depth);
}

void print_irfunc(const struct IRFunc* func)
{
    assert(func);

    printf("FUNC '%s'\n", func->ident);
    print_irlist(func->instrs, 1);

    if (func->next)
        print_irfunc(func->next);
}

void free_irlist(struct IRInstr* head)
{
    if (head->dest.type == OPRND_FUNC) {
        xfree((void**)&head->dest.func.ident);
    }
    if (head->src1.type == OPRND_FUNC) {
        xfree((void**)&head->src1.func.ident);
    }
    if (head->src2.type == OPRND_FUNC) {
        xfree((void**)&head->src2.func.ident);
    }
    if (head->next != NULL)
        free_irlist(head->next);

    xfree((void**)&head);
}

void free_irfunc(struct IRFunc* func)
{
    xfree((void**)&func->ident);
    free_stackframe(&func->sf);
    free_irlist(func->instrs);
    if (func->next)
        free_irfunc(func->next);
    xfree((void**)&func);
}
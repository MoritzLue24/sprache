#include "gen/irinstr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils/xalloc.h"


const char* irop_str(enum IROp op)
{
    switch (op) {
        case IR_INIT_SF:
            return "INIT_SF";
        case IR_FREE_SF:
            return "FREE_SF";
        case IR_IMM:
            return "IMM";
        case IR_ADD:
            return "ADD";
        case IR_SUB:
            return "SUB";
        case IR_MUL:
            return "MUL";
        case IR_RETURN:
            return "RETURN";
        case IR_LOAD_LOCAL:
            return "LOAD_LOCAL";
        case IR_STORE_LOCAL:
            return "STORE_LOCAL";
        default:
            assert(0);
    }
}

char* oprnd_str(struct IROperand oprnd)
{
    if (oprnd.none) {
        char* val = xmalloc(strlen("<none>") + 1);
        snprintf(val, strlen("<none>") + 1, "<none>");
        return val;
    }
    switch (oprnd.type) {
        case OPRND_VAR:
        {
            int len = snprintf(NULL, 0, "<%i>", oprnd.sf_offset);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "<%i>", oprnd.sf_offset); // \0 automatically set
            return str;
        }

        case OPRND_IMM:
        {
            int len = snprintf(NULL, 0, "[%i]", oprnd.imm);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "[%i]", oprnd.imm); // \0 automatically set
            return str;
        }

        case OPRND_VREG:
        {
            int len = snprintf(NULL, 0, "vr%zu", oprnd.vreg_i);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "vr%zu", oprnd.vreg_i); // \0 automatically set
            return str;
        }

        case OPRND_PREG:
        {
            int len = snprintf(NULL, 0, "r%zu", oprnd.preg_i);
            char* str = xmalloc(len + 1);
            snprintf(str, len + 1, "r%zu", oprnd.preg_i); // \0 automatically set
            return str;
        }

        default:
            assert(0);
    }
}

void print_irlist(struct IRInstr* head)
{
    assert(head);

    char* dest = oprnd_str(head->dest);
    char* src1 = oprnd_str(head->src1);
    char* src2 = oprnd_str(head->src2);

    // printf("%s\tdest=%s\t\tsrc1=%s\t\tsrc2=%s\t\timm=%s\n", irop_str(head->op), dest, src1, src2, imm);

    switch (head->op) {
        case IR_INIT_SF:
            printf("INIT_SF, %s\n", src1);
            break;
        case IR_FREE_SF:
            printf("FREE_SF, %s\n", src1);
            break;
        case IR_IMM:
            printf("%s <- %s\n", dest, src1);
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
        print_irlist(head->next);
}

void free_irlist(struct IRInstr* head)
{
    if (head->next != NULL)
        free_irlist(head->next);
    xfree((void**)&head);
}
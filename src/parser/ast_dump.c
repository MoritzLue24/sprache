#include "parser/ast.h"

#define TABSIZE 4

static void node_dump_depth(
    const char* label, const struct Node* n, FILE* out, int depth
);
static void node_dump_all_depth(
    const char* label, const struct NodeList* nl, FILE* out, int depth
);

void node_dump(const struct Node* n, FILE* out)
{
    node_dump_depth(NULL, n, out, 0);
}

void node_dump_all(const struct NodeList* nl, FILE* out)
{
    node_dump_all_depth(NULL, nl, out, 0);
}

static void node_dump_depth(
    const char* label, const struct Node* n, FILE* out, int depth
) {
    int fd = depth + 1;

    fprintf(out, "%*s", depth * TABSIZE, "");
    if (label != NULL) fprintf(out, "%s: ", label);
    if (n == NULL) {
        fprintf(out, "NULL\n");
        return;
    }
    fprintf(out, "%s {\n", node_kind_str(n->kind));

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "loc: %u:%u\n", n->loc.line, n->loc.col);

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "op: %s\n", op_kind_str(n->op));

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "value: %s\n", n->value);

    switch (n->kind) {
        case NODE_PROGRAM:
            node_dump_all_depth("nl", &n->program.nl, out, fd);
            break;
        case NODE_FUNC_DEF:
            node_dump_all_depth("params", &n->func_def.params, out, fd);
            node_dump_depth("body", n->func_def.body, out, fd);
            break;
        case NODE_BLOCK:
            node_dump_all_depth("nl", &n->block.nl, out, fd);
            break;
        case NODE_VAR_DEF:
            node_dump_depth("init", n->var.init, out, fd);
            break;
        case NODE_RETURN:
            node_dump_depth("expr", n->ret.expr, out, fd);
            break;
        case NODE_BINARY:
            node_dump_depth("lhs", n->binary.lhs, out, fd);
            node_dump_depth("rhs", n->binary.rhs, out, fd);
            break;
        case NODE_UNARY:
            node_dump_depth("operand", n->unary.operand, out, fd);
            break;
        case NODE_CALL:
            node_dump_all_depth("args", &n->call.args, out, fd);
            break;
        case NODE_BUILTIN:
            node_dump_all_depth("args", &n->builtin.args, out, fd);
            break;

        // These nodes does not have union fields, no need to dump them
        case NODE_PARAM:
        case NODE_VAR_DECL:
        case NODE_LITERAL:
        case NODE_IDENT:
            break;
    }
    fprintf(out, "%*s", depth * TABSIZE, "");
    fprintf(out, "}\n");
}

static void node_dump_all_depth(
    const char* label, const struct NodeList* nl, FILE* out, int depth
) {
    fprintf(out, "%*s", depth * TABSIZE, "");
    if (label != NULL) fprintf(out, "%s: ", label);
    fprintf(out, "[\n");

    for (size_t i = 0; i < nl->count; i++) {
        node_dump_depth(NULL, nl->items[i], out, depth + 1);
    }
    fprintf(out, "%*s", depth * TABSIZE, "");
    fprintf(out, "]\n");
}
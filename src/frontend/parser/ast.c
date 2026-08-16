#include "frontend/parser/ast.h"

#include <stdio.h>
#include <assert.h>

#include "utils/xalloc.h"
#include "frontend/sema/symbols.h"
#include "frontend/sema/builtins.h"



struct Node* alloc_node(enum NodeType type, struct Loc begin)
{
    struct Node* n = xmalloc(sizeof(struct Node));
    n->type = type;
    n->begin = begin;
    n->next = NULL;
    return n;
}

struct Node* alloc_invalid_node()
{
    return alloc_node(NODE_INVALID, EMPTY_LOC);
}

struct Node* push_node(struct Node* head, struct Node* node)
{
    struct Node* real_head = head;
    struct Node* tail = NULL;

    for (; head != NULL && head->next != NULL; head = head->next);
    tail = head;

    if (tail == NULL) {
        return node;
    }
    tail->next = node;
    return real_head;
}

size_t nodelist_size(const struct Node* head)
{
    size_t i = 0;
    for (; head != NULL; head = head->next) {i++;}
    return i;
}

enum OpType tt_to_op(enum TokenType tt)
{
    switch (tt) {
        case TT_PLUS:
            return OP_PLUS;
        case TT_MINUS:
            return OP_MINUS;
        case TT_STAR:
            return OP_MUL;
        case TT_BW_NOT:
            return OP_BW_NOT;
        case TT_BW_AND:
            return OP_BW_AND;
        case TT_BW_OR:
            return OP_BW_OR;
        case TT_BW_XOR:
            return OP_BW_XOR;

        default:
            assert(0);
    }
}

const char* op_type_str(enum OpType type)
{
    switch (type) {
        case OP_PLUS:  return "PLUS";
        case OP_MINUS: return "MINUS";
        case OP_MUL:   return "MUL";
        case OP_BW_NOT: return "BW_NOT";
        case OP_BW_AND: return "BW_AND";
        case OP_BW_OR: return "BW_OR";
        case OP_BW_XOR: return "BW_XOR";
        default: assert(0);
    }
}

static void print_indent(int depth)
{
    printf("%*s", depth * 4, "");
}

static void print_str_field(const char* label, const char* value, int depth)
{
    print_indent(depth);
    printf("%s: '%s'\n", label, value);
}

static void print_nodelist(const char* label, const struct Node* nl_head, int depth)
{
    print_indent(depth);
    if (nl_head == NULL) {
        printf("%s: []\n", label);
        return;
    }

    printf("%s: [\n", label);
    for (const struct Node* cur = nl_head; cur != NULL; cur = cur->next) {
        print_node(NULL, cur, depth + 1);
    }
    print_indent(depth);
    printf("]\n");
}

void print_node(const char* label, const struct Node* node, int depth)
{
    assert(node != NULL);
    int field_depth = depth + 1;

    print_indent(depth);
    if (label != NULL) {
        printf("%s: ", label);
    }

	switch (node->type) {
        case NODE_INVALID:
            printf("INVALID\n");
            break;

        case NODE_PROGRAM:
            printf("PROGRAM {\n");
            print_nodelist("items", node->program.items_head, field_depth);
            break;

        case NODE_FUNC_DEF:
            printf("FUNC_DEF {\n");
            print_str_field("ident", node->func_def.ident, field_depth);
            print_symbol("symbol", node->func_def.symbol, field_depth);
            print_nodelist("params", node->func_def.params_head, field_depth);
            print_node("body", node->func_def.body, field_depth);
            break;

        case NODE_PARAM:
            printf("PARAM {\n");
            print_str_field("ident", node->param.ident, field_depth);
            print_symbol("symbol", node->param.symbol, field_depth);
            break;

        case NODE_BLOCK:
            printf("BLOCK {\n");
            print_nodelist("statements", node->block.statements_head, field_depth);
            break;

        case NODE_VAR_DECL:
            printf("VAR_DECL {\n");
            print_node("target", node->var_decl.target, field_depth);
            break;

        case NODE_VAR_DEF:
            printf("VAR_DEF {\n");
            print_node("target", node->var_def.target, field_depth);
            print_node("expr", node->var_def.expr, field_depth);
            break;

        case NODE_ASSIGN_EXPR:
            printf("ASSIGN_EXPR {\n");
            print_node("target", node->assign_expr.target, field_depth);
            print_node("expr", node->assign_expr.expr, field_depth);
            break;

        case NODE_RETURN:
            printf("RETURN {\n");
            print_node("expr", node->ret.expr, field_depth);
            break;

        case NODE_VAR:
            printf("VAR {\n");
            print_str_field("ident", node->var.ident, field_depth);
            print_symbol("symbol", node->var.symbol, field_depth);
            break;

        case NODE_CALL:
            printf("CALL {\n");
            print_str_field("ident", node->call.ident, field_depth);
            print_symbol("symbol", node->call.symbol, field_depth);
            print_nodelist("args", node->call.args_head, field_depth);
            break;

        case NODE_UNARY_OP:
            printf("UNARY_OP {\n");
            print_str_field("op", op_type_str(node->unary_op.op), field_depth);
            print_node("factor", node->unary_op.factor, field_depth);
            break;

		case NODE_BINARY_OP:
			printf("BINARY_OP {\n");
            print_str_field("op", op_type_str(node->bin_op.op), field_depth);
            print_node("lhs", node->bin_op.lhs, field_depth);
            print_node("rhs", node->bin_op.rhs, field_depth);
			break;

		case NODE_LITERAL:
			printf("LITERAL {\n");
            print_str_field("value", node->literal.value, field_depth);
			break;

        case NODE_BUILTIN:
            printf("BUILTIN {\n");
            print_str_field("ident", node->builtin.ident, field_depth);
            print_builtin_def("def", node->builtin.def, field_depth);
            print_nodelist("args", node->builtin.args_head, field_depth);
            break;

		default:
			assert(0);
	}

    if (node->type != NODE_INVALID) {
        print_indent(depth);
        printf("}\n");
    }
}

void free_node(struct Node* node)
{
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE_INVALID:
            break;

        case NODE_PROGRAM:
            free_node(node->program.items_head);
            break;

        case NODE_FUNC_DEF:
            xfree((void**)&node->func_def.ident);
            free_node(node->func_def.params_head);
            free_node(node->func_def.body);
            break;

        case NODE_PARAM:
            xfree((void**)&node->param.ident);
            break;

        case NODE_BLOCK:
            free_node(node->block.statements_head);
            break;

        case NODE_VAR_DECL:
            free_node(node->var_decl.target);
            break;

        case NODE_VAR_DEF:
            free_node(node->var_def.target);
            free_node(node->var_def.expr);
            break;

        case NODE_ASSIGN_EXPR:
            free_node(node->assign_expr.target);
            free_node(node->assign_expr.expr);
            break;

        case NODE_RETURN:
            free_node(node->ret.expr);
            break;

        case NODE_VAR:
            xfree((void**)&node->var.ident);
            break;

        case NODE_CALL:
            xfree((void**)&node->call.ident);
            free_node(node->call.args_head);
            break;

        case NODE_UNARY_OP:
            free_node(node->unary_op.factor);
            break;

        case NODE_BINARY_OP:
            free_node(node->bin_op.lhs);
            free_node(node->bin_op.rhs);
            break;

        case NODE_LITERAL:
            xfree((void**)&node->literal.value);
            break;

        case NODE_BUILTIN:
            xfree((void**)&node->builtin.ident);
            free_node(node->builtin.args_head);
            break;

        default:
            assert(0);
    }

    if (node->next != NULL) {
        free_node(node->next);
    }

    xfree((void**)&node);
}

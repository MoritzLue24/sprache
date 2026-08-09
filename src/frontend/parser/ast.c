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
    return n;
}

struct Node* alloc_invalid_node()
{
    return alloc_node(NODE_INVALID, EMPTY_LOC);
}

static void expand_nodelist(struct NodeList* nl)
{
    nl->capacity *= 2;
    nl->data = xrealloc(nl->data, nl->capacity, sizeof(struct Node*));
}

void init_nodelist(struct NodeList* nl)
{
    nl->data = calloc(NODELIST_INIT_CAPACITY, sizeof(struct Node*));
    nl->size = 0;
    nl->capacity = NODELIST_INIT_CAPACITY;
}

bool nodelist_push(struct NodeList* nl, struct Node* node)
{
    if (nl->head + nl->size >= nl->capacity)
        expand_nodelist(nl);

    nl->data[nl->size++] = node;
    return true;
}

void free_nodelist(struct NodeList* nl)
{
    for (size_t i = 0; i < nl->size; i++) {
        free_node(nl->data[i]);
    }
    xfree((void**)&nl->data);
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

static void print_nodelist(const char* label, const struct NodeList* nl, int depth)
{
    print_indent(depth);
    if (nl->size == 0) {
        printf("%s: []\n", label);
        return;
    }

    printf("%s: [\n", label);
    for (size_t i = 0; i < nl->size; i++) {
        print_node(NULL, nl->data[i], depth + 1);
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
            print_nodelist("items", &node->program.items, field_depth);
            break;

        case NODE_FUNC_DEF:
            printf("FUNC_DEF {\n");
            print_str_field("ident", node->func_def.ident, field_depth);
            print_symbol("symbol", node->func_def.symbol, field_depth);
            print_nodelist("params", &node->func_def.params, field_depth);
            print_node("body", node->func_def.body, field_depth);
            break;

        case NODE_PARAM:
            printf("PARAM {\n");
            print_str_field("ident", node->param.ident, field_depth);
            print_symbol("symbol", node->param.symbol, field_depth);
            break;

        case NODE_BLOCK:
            printf("BLOCK {\n");
            print_nodelist("statements", &node->block.statements, field_depth);
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
            print_nodelist("args", &node->call.args, field_depth);
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
            print_nodelist("args", &node->builtin.args, field_depth);
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
    assert(node != NULL);

    switch (node->type) {
        case NODE_INVALID:
            break;

        case NODE_PROGRAM:
            free_nodelist(&node->program.items);
            break;

        case NODE_FUNC_DEF:
            xfree((void**)&node->func_def.ident);
            free_nodelist(&node->func_def.params);
            free_node(node->func_def.body);
            break;

        case NODE_PARAM:
            xfree((void**)&node->param.ident);
            break;

        case NODE_BLOCK:
            free_nodelist(&node->block.statements);
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
            free_nodelist(&node->call.args);
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
            free_nodelist(&node->builtin.args);
            break;

        default:
            assert(0);
    }
    xfree((void**)&node);
}

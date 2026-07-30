#include "parser/ast.h"

#include <stdio.h>
#include <assert.h>

#include "utils/xalloc.h"


static void expand_nodelist(struct NodeList* nl)
{
    nl->capacity *= 2;
    nl->data = xrealloc(nl->data, nl->capacity, sizeof(struct Node*));
}

void init_nodelist(struct NodeList* nl, size_t capacity)
{
    nl->data = calloc(capacity, sizeof(struct Node*));
    nl->size = 0;
    nl->capacity = capacity;
    nl->head = 0;
}

bool nodelist_push(struct NodeList* nl, struct Node* node)
{
    if (nl->head + nl->size >= nl->capacity)
        expand_nodelist(nl);

    nl->data[nl->head + nl->size++] = node;
    return true;
}

struct Node* nodelist_pop_first(struct NodeList* nl)
{
    if (nl->size == 0)
        return NULL;

    nl->size--;
    return nl->data[nl->head++];
}

struct Node* nodelist_get(const struct NodeList* nl, size_t i)
{
    if (i >= nl->size)
        return NULL;
    return nl->data[nl->head + i];
}

void free_nodelist(struct NodeList* nl)
{
    xfree((void**)&nl->data);
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
    printf("%s: [\n", label);
    for (size_t i = 0; i < nl->size; i++) {
        print_node(NULL, nodelist_get(nl, i), depth + 1);
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
        case NODE_BLOCK:
            printf("BLOCK {\n");
            print_nodelist("nodes", &node->block.nodes, field_depth);
            break;

        case NODE_VAR_DECL:
            printf("VAR_DECL {\n");
            print_str_field("ident", node->var_decl.ident, field_depth);
            print_symbol("symbol", node->var_decl.symbol, field_depth);
            break;

        case NODE_VAR_DEF:
            printf("VAR_DEF {\n");
            print_str_field("ident", node->var_def.ident, field_depth);
            print_symbol("symbol", node->var_def.symbol, field_depth);
            print_node("expr", node->var_def.expr, field_depth);
            break;

        case NODE_VAR_ASSIGN:
            printf("VAR_ASSIGN {\n");
            print_str_field("ident", node->var_assign.ident, field_depth);
            print_symbol("symbol", node->var_assign.symbol, field_depth);
            print_node("expr", node->var_assign.expr, field_depth);
            break;

        case NODE_VAR:
            printf("VAR {\n");
            print_str_field("ident", node->var.ident, field_depth);
            print_symbol("symbol", node->var.symbol, field_depth);
            break;

        case NODE_RETURN:
            printf("RETURN {\n");
            print_node("expr", node->ret.expr, field_depth);
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
    print_indent(depth);
    printf("}\n");
}

void free_ast(struct Node* root)
{
    if (root == NULL)
        return;

    switch (root->type) {
        case NODE_BLOCK:
            for (size_t i = 0; i < root->block.nodes.size; i++) {
                free_ast(nodelist_get(&root->block.nodes, i));
            }
            // not free_queue, that would call free on all nodes. we do that already
            xfree((void**)&root->block.nodes.data);
            break;

        case NODE_VAR_DECL:
            if (root->var_decl.ident != NULL)
                xfree((void**)&root->var_decl.ident);
            break;

        case NODE_VAR_DEF:
            if (root->var_def.ident != NULL)
                xfree((void**)&root->var_def.ident);
            free_ast(root->var_def.expr);
            break;

        case NODE_VAR_ASSIGN:
            if (root->var_assign.ident != NULL)
                xfree((void**)&root->var_assign.ident);
            free_ast(root->var_assign.expr);
            break;

        case NODE_VAR:
            if (root->var.ident != NULL)
                xfree((void**)&root->var.ident);
            break;

        case NODE_RETURN:
            free_ast(root->ret.expr);
            break;

        case NODE_BINARY_OP:
            free_ast(root->bin_op.lhs);
            free_ast(root->bin_op.rhs);
            break;

        case NODE_LITERAL:
            xfree((void**)&root->literal.value);
            break;

        case NODE_BUILTIN:
            if (root->builtin.ident != NULL)
                xfree((void**)&root->builtin.ident);

            struct Node* cur = nodelist_pop_first(&root->builtin.args);
            while (cur != NULL) {
                free_ast(cur);
                cur = nodelist_pop_first(&root->builtin.args);
            }
            free_nodelist(&root->builtin.args);
            break;

        default:
            assert(0);
    }
    xfree((void**)&root);
}

#ifndef AST_H
#define AST_H

#include <stdint.h>

#include "loc.h"
#include "tokens.h"


enum NodeKind
{
	NODE_MODULE,
	NODE_FUNC_DEF,
	NODE_BLOCK,
	NODE_RETURN,
	NODE_LITERAL,
};

struct Node;
struct Node_Module;
struct Node_FuncDef;
struct Node_Block;
struct Node_Return;
struct Node_Literal;

struct Node_Module
{
	struct Node *body;	/* of kind NODE_FUNCTION_DEF */
	size_t count;
};

struct Node_FuncDef
{
	struct Token name_t;
	struct Node *block_n;	/* of kind NODE_BLOCK */
};

struct Node_Block
{
	struct Node *body;	/* of kind Node_RETURN */
	size_t count;
};

struct Node_Return 
{
	struct Node *value_n;	/* of kind NODE_LITERAL */
};

struct Node_Literal 
{
	struct Token value_t;
};

struct Node 
{
	enum NodeKind kind;
	struct Loc start, end;
	union {
		struct Node_Module n_module;
		struct Node_FuncDef n_func_def;
		struct Node_Block n_block;
		struct Node_Return n_return;
		struct Node_Literal n_literal;
	};
};

void
print_node(struct Node node, uint8_t identation);

void
free_node(struct Node node);

#endif

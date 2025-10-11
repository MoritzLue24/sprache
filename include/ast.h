#ifndef AST_H
#define AST_H

#include <stdint.h>

#include "loc.h"
#include "tokens.h"


enum NodeKind
{
	NODE_MODULE,
	NODE_FUNCTION,
	NODE_BLOCK,
	NODE_RETURN,
	NODE_LITERAL,
};

struct Node;
struct Node_Module;
struct Node_Function;
struct Node_Block;
struct Node_Return;
struct Node_Literal;

struct Node_Module
{
	struct Node *body;
	size_t count;
};

struct Node_Function
{
	struct Token name_token;
	struct Node *block_node;
};

struct Node_Block
{
	struct Node *body;
	size_t count;
};

struct Node_Return 
{
	struct Node *value_node;
};

struct Node_Literal 
{
	struct Token value_token;
};

struct Node 
{
	enum NodeKind kind;
	struct Loc start, end;
	union {
		struct Node_Module n_module;
		struct Node_Function n_function;
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

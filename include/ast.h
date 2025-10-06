#ifndef AST_H
#define AST_H

#include "loc.h"
#include "tokens.h"


enum NodeKind
{
	NODE_BLOCK,
	NODE_FUNCTION,
	NODE_RETURN,
	NODE_LITERAL,
};

struct Node;
struct Node_Block;
struct Node_Function;
struct Node_Return;
struct Node_Literal;

struct Node_Block
{
	struct Node *body;
	unsigned int count;
};

struct Node_Function
{
	struct Token name_token;
	struct Node *block_node;
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
		struct Node_Block n_block;
		struct Node_Function n_function;
		struct Node_Return n_return;
		struct Node_Literal n_literal;
	};
};

void
print_node(struct Node node, uint8_t identation);

void
free_node(struct Node node);

#endif

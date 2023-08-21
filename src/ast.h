#ifndef AST_H
#define AST_H

#include <stdint.h>


enum NodeKind
{
	NODE_ROOT,
	NODE_FUNCTION,
	NODE_RETURN,
	NODE_LITERAL,
};

struct Node;

struct Node_Root
{
	struct Node *body;
};

struct Node_Function
{
	const char *name;
};

struct Node_Return 
{
	struct Node *value;
};

struct Node_Literal 
{
	struct Node *body;
	uint32_t u_value;
};

struct Node 
{
	enum NodeKind kind;
	int32_t start_i;
	int32_t end_i;

	union {
		struct Node_Root root;
	};
};

#endif

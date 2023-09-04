#ifndef AST_H
#define AST_H

#include <stdint.h>
#include "loc.h"
#include "tokens.h"


enum NodeKind
{
	NODE_ROOT,
	NODE_FUNCTION,
	NODE_RETURN,
	NODE_LITERAL,
};

struct Node;
struct NodeListElement;
struct Node_Root;
struct Node_Function;
struct Node_Return;
struct Node_Literal;


struct NodeListElement
{
	struct Node *self;
	struct NodeListElement *next;
};

struct Node_Root
{
	struct NodeListElement *body;
};

struct Node_Function
{
	struct Token name;
	struct NodeListElement *args;
	struct NodeListElement *body;
};

struct Node_Return 
{
	struct Node *value;
};

struct Node_Literal 
{
	struct Token value;
};

struct Node 
{
	enum NodeKind kind;
	/* struct Loc start, end; */
	union {
		struct Node_Root n_root;
		struct Node_Function n_function;
		struct Node_Return n_return;
		struct Node_Literal n_literal;
	};
};

#endif

#ifndef NODES_H
#define NODES_H


enum NodeKind
{
	NODE_ROOT,
	NODE_FUNCTION,
	NODE_RETURN,
};

struct Node_Root
{
	struct Node *body;
};

struct Node_Function
{
	struct Node *body;
	const char *name;
};

struct Node_Return 
{
	struct Node *value;
};

struct Node 
{
	enum NodeKind kind;
	union {
		struct Node_Root node_root;
		struct Node_Function node_function;
	};
};

#endif /* NODES_H */

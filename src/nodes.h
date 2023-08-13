#ifndef NODES_H
#define NODES_H


enum NodeKind
{
	NODE_ROOT,
	NODE_STATEMENT,
};

struct Node_Root {};

struct Node_Statement {};

struct Node 
{
	enum NodeKind kind;
	union {
		Node_Root root;
	};
};

#endif /* NODES_H */

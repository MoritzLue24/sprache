#include "parser/ast.h"

const char* node_kind_str(enum NodeKind nk)
{
    switch (nk) {
        case NODE_INVALID: return "NODE_INVALID";
#define NODE(kind) case kind: return #kind;
#include "parser/node.def"
#undef NODE
    }
    return NULL;
}

int op_kind_binary_prec(enum OpKind op)
{
    switch (op) {
        case OP_INVALID: break;
#define BINOP(kind, tok, prec, assoc) case kind: return prec;
#include "parser/binop.def"
#undef BINOP
#define UNOP(kind, tok) case kind: break;
#include "parser/unop.def"
#undef UNOP
    }
    return -1;
}

enum AssocKind op_kind_binary_assoc(enum OpKind op)
{
    switch (op) {
        case OP_INVALID: break;
#define BINOP(kind, tok, prec, assoc) case kind: return assoc;
#include "parser/binop.def"
#undef BINOP
#define UNOP(kind, tok) case kind: break;
#include "parser/unop.def"
#undef UNOP
    }
    return ASSOC_INVALID;
}

const char* op_kind_str(enum OpKind op)
{
    switch (op) {
        case OP_INVALID: return "OP_INVALID";
#define BINOP(kind, tok, prec, assoc) case kind: return #kind;
#include "parser/binop.def"
#undef BINOP
#define UNOP(kind, tok) case kind: return #kind;
#include "parser/unop.def"
#undef UNOP
    }
    return NULL;
}

enum OpKind op_kind_binary_from_token(enum TokenKind tk)
{
#define BINOP(kind, tok, prec, assoc) if (tok == tk) return kind;
#include "parser/binop.def"
#undef BINOP
    return OP_INVALID;
}

enum OpKind op_kind_unary_from_token(enum TokenKind tk)
{
#define UNOP(kind, tok) if (tok == tk) return kind;
#include "parser/unop.def"
#undef UNOP
    return OP_INVALID;
}

void node_init(
    struct Node* n, enum NodeKind kind, struct SourceLoc loc, enum OpKind op,
    const char* value
) {
    *n = (struct Node){ 0 };
    n->kind = kind;
    n->loc = loc;
    n->op = op;
    n->value = value;

    n->type = TYPE_INVALID;
    n->symbol = NULL;
}

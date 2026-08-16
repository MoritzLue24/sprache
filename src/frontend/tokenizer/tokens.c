#include "frontend/tokenizer/tokens.h"

#include <stdio.h>
#include <assert.h>

#include "utils/xalloc.h"


const struct MatchTypePair punctuations[] =
{
    {";", TT_SEMICOLON},
    {",", TT_COMMA},
    {"@", TT_AT},
    {"{", TT_LBRACE},
    {"}", TT_RBRACE},
    {"=", TT_EQ},
    {"(", TT_LPAREN},
    {")", TT_RPAREN},
	{"+", TT_PLUS},
    {"-", TT_MINUS},
    {"*", TT_STAR},
    {"&", TT_BW_AND},
    {"|", TT_BW_OR},
    {"~", TT_BW_NOT},
    {"^", TT_BW_XOR},
    {"==", TT_EQEQ},
    {"!=", TT_NEQ},
    {"<", TT_LT},
    {">", TT_GT},
    {"<=", TT_LE},
    {">=", TT_GE}
    /*
    {"&&", TT_AND},
    {"||", TT_OR},
    {"!", TT_NOT}
    */
};
const size_t punctuations_count = sizeof(punctuations) / sizeof((punctuations)[0]);

const struct MatchTypePair keywords[] =
{
    {"fn", TT_FUNC},
    {"var", TT_VAR},
    {"return", TT_RETURN}
};
const size_t keywords_count = sizeof(keywords) / sizeof((keywords)[0]);


const char* tt_str(enum TokenType tt)
{
    switch (tt)
    {
        case TT_INVALID:
            return "INVALID";
        case TT_END:
            return "END";
        case TT_IDENT:
            return "IDENT";
        case TT_LITERAL:
            return "LITERAL";
        case TT_FUNC:
            return "FUNC";
        case TT_RETURN:
            return "RETURN";
        case TT_VAR:
            return "VAR";
        case TT_SEMICOLON:
            return "SEMICOLON";
        case TT_COMMA:
            return "COMMA";
        case TT_AT:
            return "AT";
        case TT_LBRACE:
            return "LBRACE";
        case TT_RBRACE:
            return "RBRACE";
        case TT_EQ:
            return "EQ";
        case TT_LPAREN:
            return "LPAREN";
        case TT_RPAREN:
            return "RPAREN";
        case TT_PLUS:
            return "PLUS";
        case TT_MINUS:
            return "MINUS";
        case TT_STAR:
            return "STAR";
        case TT_EQEQ:
            return "EQEQ";
        case TT_NEQ:
            return "NEQ";
        case TT_LT:
            return "LT";
        case TT_LE:
            return "LE";
        case TT_GT:
            return "GT";
        case TT_GE:
            return "GE";
        case TT_BW_AND:
            return "BW_AND";
        case TT_BW_OR:
            return "BW_OR";
        case TT_BW_NOT:
            return "BW_NOT";
        case TT_BW_XOR:
            return "BW_XOR";
        default:
            assert(0);
    }
}

static void print_tokenlist_rec(struct Token* head)
{
    assert(head != NULL);
    if (head->value != NULL)
        printf("%-15s %5i:%-5i '%s'\n", tt_str(head->type), head->begin.ln, head->begin.col, head->value);
    else
        printf("%-15s %5i:%-5i\n", tt_str(head->type), head->begin.ln, head->begin.col);
    if (head->next != NULL)
        print_tokenlist_rec(head->next);
}

void print_tokenlist(struct Token* head)
{
    printf("%-15s %5s:%-5s %s\n", "type", "line", "col", "value");
    print_tokenlist_rec(head);
}

void free_tokenlist(struct Token* head)
{
    if (head->value != NULL)
        xfree((void**)&head->value);
    if (head->next != NULL)
        free_tokenlist(head->next);
    xfree((void**)&head);
}
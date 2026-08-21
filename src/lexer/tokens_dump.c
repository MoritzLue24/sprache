#include "lexer/tokens.h"

void dump_token(const struct Token* tok, FILE* out)
{
    fprintf(
        out, "%-15s %5i:%-5i '%s'\n", token_kind_str(tok->kind), tok->loc.line,
        tok->loc.col, tok->value
    );
}

void dump_all_tokens(const struct TokenList* tokl, FILE* out)
{
    for (size_t i = 0; i < tokl->count; i++) {
        dump_token(&tokl->items[i], out);
    }
}

#include "lexer/tokens.h"

void token_dump(const struct Token* tok, FILE* out)
{
    fprintf(
        out, "%-15s %5u:%-5u '%s'\n", token_kind_str(tok->kind), tok->loc.line,
        tok->loc.col, tok->value
    );
}

void token_dump_all(const struct TokenList* tokl, FILE* out)
{
    for (size_t i = 0; i < tokl->count; i++) {
        token_dump(&tokl->items[i], out);
    }
}

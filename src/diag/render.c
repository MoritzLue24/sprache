#include "sprache/diag.h"

void diag_render(const struct Diag* d, FILE* out)
{
    const char* codestr = diag_code_str(d->code);
    fprintf(out, "%s at %i:%i: '%s'\n",
        codestr, d->loc.line, d->loc.col, d->message);
}

void diag_render_all(const struct DiagList* dl, FILE* out)
{
    for (size_t i = 0; i < dl->count; i++) {
        diag_render(&dl->items[i], out);
    }
}
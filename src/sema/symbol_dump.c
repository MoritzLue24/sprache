#include "sema/symbols.h"
#include "type/type.h"
#include "builtin/builtin.h"

#define TABSIZE 4

void symbol_dump(
    const char* label, const struct Symbol* sym, FILE* out, int depth
) {
    int fd = depth + 1;

    fprintf(out, "%*s%s: Symbol(\n", depth * TABSIZE, "", label);

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "kind: %s\n", symbol_kind_str(sym->kind));

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "name: %s\n", sym->name);

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "type: %s\n", type_kind_str(sym->type));
    
    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "loc_decl: %u:%u\n", sym->loc_decl.line, sym->loc_decl.col);

    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "param_count: %zu\n", sym->param_count);
    
    fprintf(out, "%*sparams: [", fd * TABSIZE, "");
    for (size_t i = 0; i < sym->param_count; i++) {
        if (i == sym->param_count - 1) {
            fprintf(out, "%s", type_kind_str(sym->params[i]));
        }
        else fprintf(out, "%s, ", type_kind_str(sym->params[i]));
    }
    fprintf(out, "]\n");
    
    fprintf(out, "%*s", fd * TABSIZE, "");
    fprintf(out, "builtin_kind: %s\n", builtin_kind_str(sym->builtin_kind));

    fprintf(out, "%*s)\n", depth * TABSIZE, "");
}
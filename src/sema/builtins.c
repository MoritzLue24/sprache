#include "sema/builtins.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>


const struct BuiltinDef builtins[] =
{
    {BUILTIN_READ_PIN, "read", 1},
};
const size_t builtins_count = sizeof(builtins) / sizeof((builtins)[0]);


const struct BuiltinDef* match_builtin(const char* ident)
{
    for (size_t i = 0; i < builtins_count; i++) {
        if (strcmp(ident, builtins[i].ident) == 0) {
            return &builtins[i];
        }
    }
    return NULL;
}

static const char* builtin_type_str(enum BuiltinType type)
{
    switch (type) {
        case BUILTIN_READ_PIN: return "READ_PIN";
        default: assert(0);
    }
}

void print_builtin_def(const char* label, const struct BuiltinDef* def, int depth)
{
    printf("%*s", depth * 4, "");
    if (label != NULL) {
        printf("%s: ", label);
    }

    if (def == NULL) {
        printf("<unresolved>\n");
        return;
    }

    printf("BuiltinDef (\n");

    printf("%*s", (depth + 1) * 4, "");
    printf("type: '%s'\n", builtin_type_str(def->type));

    printf("%*s", (depth + 1) * 4, "");
    printf("ident: '%s'\n", def->ident);

    printf("%*s", (depth + 1) * 4, "");
    printf("arg_count: %li\n", def->arg_count);

    printf("%*s", depth * 4, "");
    printf(")\n");
}
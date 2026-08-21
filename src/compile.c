#include "sprache/compile.h"
#include "lexer/lexer.h"
#include "utils/str.h"
#include "utils/darray.h"
#include <string.h>

enum SpracheStage sprache_stage_from_str(char* s)
{
    if (strcmp(strupper(s), "INVALID") == 0) {
        return SPRACHE_STAGE_INVALID;
    }
#define STAGE(name, spellig, file_ext) \
    if (strcmp(strupper(s), spellig) == 0) { \
        return name; \
    }
#include "sprache/stage.def"
#undef STAGE
    return SPRACHE_STAGE_INVALID;
}

const char* sprache_stage_get_file_ext(enum SpracheStage stage)
{
    switch (stage) {
        case SPRACHE_STAGE_INVALID:
            return NULL;
#define STAGE(name, spellig, file_ext) case name: return file_ext;
#include "sprache/stage.def"
#undef STAGE
    }
    return NULL;
}

struct CompileResult sprache_compile(
    struct Arena* a, struct CompileOptions options
) {
    struct CompileResult res = { .ok = true };
    DARRAY_INIT(a, &res.diags, 10);

    struct TokenList tkl = lex(a, options.source);
    if (options.stop_after == SPRACHE_STAGE_TOKENS) {
        dump_all_tokens(&tkl, options.out);
        return res;
    }

    return res;
}

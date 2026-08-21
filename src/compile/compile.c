#include "sprache/compile.h"
#include "lexer/lexer.h"
#include "utils/darray.h"
#include "diag/diag_internal.h"

struct CompileResult sprache_compile(
    struct Arena* a, struct CompileOptions options
) {
    struct CompileResult res = { .ok = true };
    DARRAY_INIT(a, &res.diags, DIAG_INIT_CAPACITY);

    struct TokenList tkl = lex(a, options.source);
    if (options.stop_after == SPRACHE_STAGE_TOKENS) {
        token_dump_all(&tkl, options.out);
        goto done;
    }

done:
    if (diag_has_errors(&res.diags)) {
        res.ok = false;
    }
    return res;
}

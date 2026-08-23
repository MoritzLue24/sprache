#include "sprache/compile.h"
#include "diag/diag_internal.h"
#include "utils/darray.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

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

    struct Node root = parse(a, &tkl, &res.diags);
    if (options.stop_after == SPRACHE_STAGE_AST) {
        node_dump(&root, options.out);
        goto done;
    }

done:
    if (diag_has_errors(&res.diags)) {
        res.ok = false;
    }
    return res;
}

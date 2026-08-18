#ifndef COMPILE_H
#define COMPILE_H

#include <stdio.h>
#include "sprache/diag.h"

enum SpracheStage {
    SPRACHE_STAGE_TOKENS,
    SPRACHE_STAGE_AST,
    SPRACHE_STAGE_SEMA,
    SPRACHE_STAGE_IR,
    SPRACHE_STAGE_ASM
};

struct CompileOptions {
    const char*       source;
    enum SpracheStage stop_after;
    FILE*             out;
};

struct CompileResult {
    bool            ok;
    struct DiagList diags;
};

struct CompileResult sprache_compile(struct CompileOptions options);
void                 sprache_free_result(struct CompileResult* r);

#endif
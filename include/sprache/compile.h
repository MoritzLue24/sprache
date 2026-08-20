#ifndef COMPILE_H
#define COMPILE_H

#include <stdio.h>
#include "sprache/diag.h"

enum SpracheStage {
    SPRACHE_STAGE_INVALID,
#define STAGE(name, spellig, file_ext) name,
#include "stage.def"
#undef STAGE
};

/// @brief Converts 's' to the according 'str' of 'stage.def'.
/// @note UPPERS s
enum SpracheStage sprache_stage_from_str(char* s);
const char* sprache_stage_get_file_ext(enum SpracheStage stage);

struct CompileOptions {
    const char* source;
    enum SpracheStage stop_after;
    FILE* out;
};

struct CompileResult {
    bool ok;
    struct DiagList diags;
};

struct Arena;

struct CompileResult
sprache_compile(struct Arena* a, struct CompileOptions options);

#endif
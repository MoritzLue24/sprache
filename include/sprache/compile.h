#ifndef COMPILE_H
#define COMPILE_H

#include "sprache/diag.h"
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

enum SpracheStage {
    SPRACHE_STAGE_INVALID,
#define STAGE(name, spellig, file_ext) name,
#include "sprache/stage.def"
#undef STAGE
};

/// @brief Converts 's' to the according 'str' of 'stage.def'.
/// @note UPPERS s
enum SpracheStage sprache_stage_from_str(char* s);

/// @brief Get the file extension, related to the stage,
/// of the resulting "out" file (includes the dot)
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

struct CompileResult sprache_compile(
    struct Arena* a, struct CompileOptions options
);

#endif

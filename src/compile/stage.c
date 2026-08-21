#include "sprache/compile.h"
#include "utils/str.h"
#include <string.h>

enum SpracheStage sprache_stage_from_str(char* s)
{
    if (strcmp(str_upper(s), "INVALID") == 0) {
        return SPRACHE_STAGE_INVALID;
    }
#define STAGE(name, spellig, file_ext) \
    if (strcmp(str_upper(s), spellig) == 0) { \
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

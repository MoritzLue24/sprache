#include "args.h"
#include <stdio.h>
#include <string.h>
#include "utils/xalloc.h"
#include "utils/str.h"

bool parse_args(struct Arguments* args, struct Arena* a, int argc, char** argv)
{
    args->show_help = false;
    args->input_file = NULL;
    args->out_file = NULL;
    args->std_out = false;
    args->sprache_stage = SPRACHE_STAGE_INVALID;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = 1;
            return true;
        }
        else if (strcmp(argv[i], "--stdout") == 0) {
            args->std_out = true;
        }
        else if (strcmp(argv[i], "-o") == 0 ||
                 strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                fprintf(
                    stderr, "Error: Argument expected after '%s'\n", argv[i]
                );
                return false;
            }
            args->out_file = argv[++i];
        }
        else if (strcmp(argv[i], "-s") == 0 ||
                 strcmp(argv[i], "--stage") == 0) {
            if (i + 1 >= argc) {
                fprintf(
                    stderr, "Error: Argument expected after '%s'\n", argv[i]
                );
                return false;
            }
            args->sprache_stage = sprache_stage_from_str(argv[++i]);
            if (args->sprache_stage == SPRACHE_STAGE_INVALID) {
                fprintf(stderr, "Error: Invalid sprache stage '%s'\n", argv[i]);
                return false;
            }
        }
        else if (args->input_file == NULL)
            args->input_file = argv[i];
        else {
            fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
            return false;
        }
    }

    if (args->input_file == NULL) {
        fprintf(stderr, "Error: Argument 'file' missing\n");
        return false;
    }
    if (!ends_with(args->input_file, ".s")) {
        fprintf(stderr, "Error: Invalid file format, '.s' expected\n");
        return false;
    }
    if (args->sprache_stage == SPRACHE_STAGE_INVALID) {
        args->sprache_stage = SPRACHE_STAGE_ASM;
    }
    if (!args->out_file) {
        const char* ext = sprache_stage_get_file_ext(args->sprache_stage);
        args->out_file = replace_last(a, args->input_file, ".s", ext);
    }
    return true;
}

void print_help(FILE* out)
{
    fprintf(
        out, "Usage: sprache <file> [options]\n\n"
             "Options:\n"
             "\t-h, --help\t\t\tShows this message\n"
             "\t-o, --output <out-file>\t\tSpecifies the output-file. If "
             "not supplied, <file>.tok/ast/ir/asm/..\n"
             "\t-s, --stage <stage>\t\tSpecifies the stage after which the "
             "compiler should stop\n"
             "\t\t\t\t\tAvailable stages: "
    );
#define STAGE(name, str, file_ext) fprintf(out, str ", ");
#include "sprache/stage.def"
#undef STAGE
    fprintf(
        out, "(default: ASM)\n"
             "\t--stdout\t\t\tRedirects the output to the command-line\n"
    );
}
#include "args.h"

#include <stdio.h>
#include <string.h>

#include "utils/str.h"
#include "utils/xalloc.h"


bool parse_args(Arguments* args, int argc, char** argv)
{
    args->show_help = 0;
    args->input_file = NULL;
    args->asm_file = NULL;
    args->asm_file_specified = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            args->show_help = 1;
            return true;
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--asm-file") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Argument expected after '%s'\n", argv[i]);
                return false;
            }
            args->asm_file = argv[++i];
            args->asm_file_specified = true;
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
        fprintf(stderr, "Invalid file format, '.s' expected\n");
        return false;
    }
    return true;
}

void same_asm_file(Arguments* args)
{
    args->asm_file = replace_last(args->input_file, ".s", ".asm");
}

void free_args(Arguments args)
{
    if (!args.asm_file_specified)
        xfree((void**)&args.asm_file);
}
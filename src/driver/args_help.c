#include "driver/args.h"
#include <stdio.h>

void args_print_help()
{
    printf(
        "Usage: sprache <file> [options]\n\n"
        "Options:\n"
        "\t-h, --help\t\t\tShows this message\n"
        "\t-o, --output <out-file>\t\tSpecifies the output-file. If "
        "not supplied, <file>.tok/ast/ir/asm/..\n"
        "\t-s, --stage <stage>\t\tSpecifies the stage after which the "
        "compiler should stop\n"
        "\t\t\t\t\tAvailable stages: "
    );

#define STAGE(name, str, file_ext) printf(str ", ");
#include "sprache/stage.def"
#undef STAGE

    printf(
        "(default: ASM)\n"
        "\t--stdout\t\t\tRedirects the output to the command-line\n"
    );
}

#include "driver/args.h"
#include "sprache/compile.h"
#include "utils/file.h"
#include "utils/arena.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    int ret = 0;
    struct Arena a;
    arena_init(&a);

    struct Arguments args;
    if (!args_parse(&a, &args, argc, argv)) {
        fprintf(stderr, "Try '--help' for more information\n");
        ret = 1;
        goto done;
    }
    if (args.show_help) {
        args_print_help();
        goto done;
    }

    struct CompileOptions opt = {
        .source = file_read(&a, args.input_file),
        .stop_after = args.sprache_stage,
        .out = args.std_out ? stdout : file_openw(args.out_file),
    };

    struct CompileResult res = sprache_compile(&a, opt);
    if (!res.ok) {
        diag_dump_all(&res.diags, stderr);
        ret = 1;
    }

    if (opt.out != stdout) {
        fclose(opt.out);
    }
done:
    arena_free(&a);
    return ret;
}

#include <stdio.h>
#include "driver/args.h"
#include "sprache/compile.h"
#include "utils/file.h"

int main(int argc, char** argv)
{
    int ret = 0;
    struct Arena a;
    init_arena(&a);

    struct Arguments args;
    if (!parse_args(&args, &a, argc, argv)) {
        printf("Try '--help' for more information\n");
        ret = 1;
        goto done;
    }
    if (args.show_help) {
        print_help(stdout);
        goto done;
    }

    struct CompileOptions opt = {
        .source = read_file(&a, args.input_file),
        .stop_after = args.sprache_stage,
        .out = args.std_out ? stdout : openw_file(args.out_file),
    };

    struct CompileResult res = sprache_compile(&a, opt);
    if (!res.ok) {
        diag_dump_all(&res.diags, stdout);
        ret = 1;
    }

    if (opt.out != stdout) {
        fclose(opt.out);
    }
done:
    free_arena(&a);
    return ret;
}
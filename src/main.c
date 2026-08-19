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
        ret = 1;
        goto done;
    }
    if (args.show_help) {
        print_help(stdout);
        goto done;
    }

    struct CompileOptions opt = {
        .source     = read_file(&a, args.input_file),
        .stop_after = args.sprache_stage,
        .out        = args.std_out ? stdout : openw_file(args.out_file)
    };

    if (opt.out != stdout) {
        fclose(opt.out);
    }
done:
    free_arena(&a);
    return ret;
}
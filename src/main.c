#include <stdio.h>

#include "args.h"
#include "utils/file.h"
#include "utils/xalloc.h"
#include "core/error.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "sema/sema.h"
#include "sema/symbols.h"
#include "gen/regalloc.h"
#include "gen/irgen.h"
#include "gen/avrgen.h"


void args_print_help()
{
    printf(
        "Usage: sprache <file> [options]\n\n"
        "Options:\n"
        "\t-h, --help\t\t\tShows this message\n"
        "\t-a, --asm-file\t\t\tSpecifies the assembly output\n"
    );
}

int main(int argc, char** argv)
{
    Arguments args;
    if (!parse_args(&args, argc, argv)) {
        printf("Type -h, --help for more.\n");
        return 1;
    }
    if (args.show_help) {
        args_print_help();
        return 0;
    }
    if (!args.asm_file_specified) {
        same_asm_file(&args);
    }

    const char* source = read_file(args.input_file);
    if (source == NULL) {
        return 1;
    }

    struct ErrorList errors;
    init_errorlist(&errors, 10);

    // syntax & semantic
    struct Token* tok_head = lex(source);
    print_tokenlist(tok_head);
    struct Node* root = parse(tok_head, &errors);
    // print_node(NULL, root, 0);

    struct SymTable st;
    check_sema(root, &errors, &st);
    print_node(NULL, root, 0);

    // goto done_sema;

    // ir & regalloc
    struct IRInstr* head = gen_ir(root, &errors);
    regalloc(head);
    print_irlist(head);
    print_errors(&errors);

    // goto done_ir;

    // avr generation
    FILE* f = fopen(args.asm_file, "w");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    gen_avr(head, f);
    fclose(f);

done_ir: __attribute__((unused));
    free_irlist(head);
done_sema: __attribute__((unused));
    free_symtable(&st);
    free_ast(root);
    free_tokenlist(tok_head);
    free_errorlist(&errors);
    xfree((void**)&source);
    free_args(args);
}
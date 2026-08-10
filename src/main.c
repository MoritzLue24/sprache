#include <stdio.h>

#include "args.h"
#include "utils/file.h"
#include "utils/xalloc.h"
#include "frontend/core/error.h"
#include "frontend/tokenizer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/sema/sema.h"
#include "frontend/sema/symbols.h"
#include "backend/target/avr_target.h"
#include "backend/ir/irgen.h"
#include "backend/codegen/regalloc.h"
#include "backend/codegen/avrgen.h"


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
    init_errorlist(&errors);

    // syntax & semantic
    struct Token* tok_head = lex(source);
    struct Node* root = parse(tok_head, &errors);

    struct SymTable st;
    init_symtable(&st, 10);
    symtable_enter_scope(&st, 10);

    target_declare_symbols(&st);
    check_sema(root, &errors, &st);
    symtable_exit_scope(&st);

    //print_node(NULL, root, 0);
    if (has_errors(&errors)) {
        print_errors(&errors);
        goto done_sema;
    }

    // ir & regalloc
    struct IRFunc* head = gen_ir(root);
    for (const struct IRFunc* cur = head; cur != NULL; cur = cur->next) {
        regalloc(cur->instrs);
    }
    //print_irfunc(head);
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
    free_irfunc(head);
done_sema: __attribute__((unused));
    free_symtable(&st);
done_parser: __attribute__((unused));
    free_node(root);
done_lexer: __attribute__((unused));
    free_tokenlist(tok_head);
    free_errorlist(&errors);
    xfree((void**)&source);
    free_args(args);
}
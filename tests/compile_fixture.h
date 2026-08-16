#ifndef COMPILE_FIXTURE_H
#define COMPILE_FIXTURE_H

#include <stdio.h>
#include <stdlib.h>

#include "core/error.h"
#include "frontend/tokenizer/lexer.h"
#include "frontend/tokenizer/tokens.h"
#include "frontend/parser/parser.h"
#include "frontend/sema/sema.h"
#include "frontend/sema/symbols.h"
#include "backend/target/avr_target.h"
#include "backend/ir/irgen.h"
#include "backend/ir/irinstr.h"
#include "backend/codegen/regalloc.h"
#include "backend/codegen/avrgen.h"


/// @brief End-to-end: source -> tokens -> AST -> sema -> IR -> regalloc.
/// Mirrors the pipeline driven by main.c so tests using this exercise how the
/// stages are actually wired together, not each stage in isolation.
struct CompileResult {
    struct Token* tok_head;
    struct Node* root;
    struct SymTable st;
    struct ErrorList errors;
    /// @brief NULL if compilation stopped due to sema errors
    struct IRFunc* ir_head;
};

/// @brief Generates an IR, but without allocating registers.
static struct CompileResult gen_ir_from_source(const char* source)
{
    struct CompileResult r;
    init_errorlist(&r.errors);

    r.tok_head = lex(source);
    r.root = parse(r.tok_head, &r.errors);

    init_symtable(&r.st, 10);
    symtable_enter_scope(&r.st, 10);
    target_declare_symbols(&r.st);
    check_sema(r.root, &r.errors, &r.st);
    symtable_exit_scope(&r.st);

    r.ir_head = NULL;
    if (!has_errors(&r.errors))
        r.ir_head = gen_ir(r.root);
    return r;
}

static void free_compile_result(struct CompileResult* r)
{
    if (r->ir_head)
        free_irfunc(r->ir_head);
    free_symtable(&r->st);
    free_node(r->root);
    free_tokenlist(r->tok_head);
    free_errorlist(&r->errors);
}

/// @brief Runs the IR through AVR codegen and returns the emitted
/// assembly as a heap string the caller must free.
static char* gen_avr_from_result(struct CompileResult res)
{
    FILE* f = tmpfile();
    if (f == NULL)
        return NULL;

    gen_avr(res.ir_head, f);

    long len = ftell(f);
    rewind(f);
    char* buf = (char*)malloc((size_t)len + 1);
    size_t read = fread(buf, 1, (size_t)len, f);
    buf[read] = '\0';
    fclose(f);
    return buf;
}

#endif

#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>
#include <stdio.h>
#include "utils/arena.h"
#include "sprache/compile.h"

struct Arguments {
    bool show_help;
    char* input_file;
    char* out_file;
    bool std_out;
    enum SpracheStage sprache_stage;
};

/// @brief Parses the cli arguments into the `Arguments` struct.
/// If no asm file is specified, uses input file as asm file.
/// @return 0 on success, a positive int otherwise (prints error message)
bool parse_args(struct Arguments* args, struct Arena* a, int argc, char** argv);
void print_help(FILE* out);

#endif
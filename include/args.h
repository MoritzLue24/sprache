#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

typedef struct {
    char* input_file;
    char* asm_file;
    bool asm_file_specified;
    bool show_help;
} Arguments;

/// @brief Parses the cli arguments into the `Arguments` struct
/// @return 0 on success, a positive int otherwise (prints error message)
bool parse_args(Arguments* args, int argc, char** argv);

/// @brief Overwrites args.asm_file with args.input_file, but with .asm ending
void same_asm_file(Arguments* args);

void free_args(Arguments args);

#endif
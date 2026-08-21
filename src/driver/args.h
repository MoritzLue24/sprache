#ifndef ARGS_H
#define ARGS_H

#include "sprache/compile.h"
#include <stdbool.h>

struct Arena;

struct Arguments {
    bool show_help;
    char* input_file;
    char* out_file;
    bool std_out;
    enum SpracheStage sprache_stage;
};

/// @brief Parses the cli arguments into the `Arguments` struct.
/// @return true on success, false otherwise (prints error message)
bool args_parse(
    struct Arena* a, struct Arguments* args, int argc, char** argv
);
void args_print_help();

#endif

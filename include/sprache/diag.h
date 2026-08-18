#ifndef DIAG_H
#define DIAG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

struct SourceLoc {
    unsigned line;
    unsigned col;
    unsigned len;
};

enum DiagCode {
#define DIAG(name, format) name,
#include "sprache/diag.def"
#undef DIAG
};

struct Diag {
    enum DiagCode    code;
    struct SourceLoc loc;
    const char*      message;
};

struct DiagList {
    struct Diag* items;
    size_t       count;
    size_t       capacity;
};

bool        diag_has_errors(const struct DiagList* dl);
const char* diag_code_str(enum DiagCode code);

/// @brief Renders a diagnostic to the given FILE.
/// If 'source' is not NULL, renders the according line part.
void diag_render(const struct Diag* d, const char* source, FILE* out);
/// @brief Renders all diagnostics to the given FILE.
/// If 'source' is not NULL, renders the according line part.
void diag_render_all(const struct DiagList* dl, const char* source, FILE* out);

#endif
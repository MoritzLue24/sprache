#include "frontend/core/loc.h"

#include <stdio.h>  // printf


void init_loc(struct Loc* loc, const char* source)
{
    loc->source = source;
    loc->c = source[0];
    loc->i = 0;
    loc->ln = 1;
    loc->col = 1;
    if (loc->c == '\0') {
        loc->end = true;
    }
    else {
        loc->end = false;
    }
}

static bool advance_char(struct Loc* loc)
{
    if (loc->source[loc->i + 1] == '\0') {
        loc->end = true;
        loc->c = '\0';
        return false;
    }

    loc->i++;
    loc->c = loc->source[loc->i];
    return true;
}

void step(struct Loc* loc)
{
    bool nl = loc->c == '\n';

    if (!advance_char(loc)) {
        return;
    }
    if (nl) {
        loc->col = 1;
        loc->ln++;
    }
    else {
        loc->col++;
    }
}

const char* loc_ptr(const struct Loc* loc)
{
    return loc->source + loc->i;
}
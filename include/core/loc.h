#ifndef LOC_H
#define LOC_H

#include <stdlib.h>
#include <stdbool.h>


struct Loc {
	const char* source;
	char c;
	size_t i;
    unsigned int ln;
    unsigned int col;
    bool end;
};

#define EMPTY_LOC (struct Loc){ NULL, '\0', 0, 0, 0, true }

void init_loc(struct Loc* loc, const char* source);

void step(struct Loc* loc);

const char* loc_ptr(const struct Loc* loc);

#endif
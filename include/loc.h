#ifndef LOC_H
#define LOC_H

#include <stddef.h>


struct Loc 
{
	const char *source;
	char c;
	size_t i;
	size_t col;
	size_t line;
};

void
step(struct Loc *loc);

#endif

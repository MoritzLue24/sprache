#ifndef LOC_H
#define LOC_H

#include <stdint.h>


struct Loc 
{
	const char *source;
	char c;
	uint32_t i;
	uint32_t col;
	uint32_t line;
};

void
step(struct Loc *loc);

#endif

#ifndef LOC_H
#define LOC_H

#include <stddef.h>
#include <stdint.h>


struct Loc 
{
	const char *source;
	char c;
	size_t i;
	uint16_t col;
	uint16_t line;
};

void
step(struct Loc *loc);

#endif

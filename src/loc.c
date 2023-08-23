#include "loc.h"
#include "error.h"


void
step(struct Loc *loc)
{
	if (loc->c == '\0')
		fail_spr(ERROR_EOF_REACHED, *loc, "Unexpected eof");

	if (loc->c == '\n')
	{
		loc->col = 0;
		++loc->line;
	}

	++loc->col;
	++loc->i;
	loc->c = loc->source[loc->i];
}

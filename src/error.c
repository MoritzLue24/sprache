#include "error.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


static const char *error_codes[] = {
	"ERROR_NONE",
	"ERROR_MEMORY_ALLOCATION",
	"ERROR_CLI_USAGE",
	"ERROR_EOF_REACHED",
	"ERROR_TOKEN_INVALID",
	"ERROR_SYNTAX_INVALID",
	"ERROR_NODE_INVALID"
};

void
fail(enum ErrorCode code, const char *msg)
{
	if (msg == NULL)
		fprintf(stderr, "%s", error_codes[code]);
	else 
		fprintf(stderr, "%s: '%s'", error_codes[code], msg);
	exit(code);
}

void
fail_spr(enum ErrorCode code, struct Loc loc, const char *msg) 
{
	if (msg == NULL)
		fprintf(stderr, 
			"Sprache error %s at %i:%i",
			error_codes[code], loc.line, loc.col);
	else 
		fprintf(stderr,
			"Sprache error %s at %i:%i: '%s'",
			error_codes[code], loc.line, loc.col, msg);
	exit(code);
}

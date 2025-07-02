#include "error.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>


static const char *error_codes[] = {
	"ERROR_NONE",
	"ERROR_CLI_USAGE",
	"ERROR_FILEPATH_INVALID",
	"ERROR_MEMORY_ALLOCATION",
	"ERROR_NOT_IMPLEMENTED",
	"ERROR_EOF_REACHED",
	"ERROR_TOKEN_INVALID",
	"ERROR_SYNTAX_INVALID",
	"ERROR_NODE_INVALID"
};

void
fail(enum ErrorCode code, const char *msg, ...)
{
	if (msg == NULL)
	{
		if (code == ERROR_CLI_USAGE)
			fprintf(stderr, "%s\nTry 'sprache --help' or 'sprache -h' for usage\n", error_codes[code]);
		else
			fprintf(stderr, "%s\n", error_codes[code]);
	}
	else
	{
		fprintf(stderr, "%s: ", error_codes[code]);

		va_list args;
    	va_start(args, msg);
		vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");
		va_end(args);

		if (code == ERROR_CLI_USAGE)
			fprintf(stderr, "Try 'sprache --help' or 'sprache -h'\n");
	}
	exit(code);
}

void
fail_spr(enum ErrorCode code, struct Loc loc, const char *msg, ...) 
{
	if (msg == NULL)
		fprintf(stderr, 
			"Sprache error %s at %i:%i",
			error_codes[code], loc.line, loc.col);
	else  
	{
		fprintf(stderr,
			"Sprache error %s at %i:%i: ",
			error_codes[code], loc.line, loc.col);
	
		va_list args;
    	va_start(args, msg);
		vfprintf(stderr, msg, args);
        fprintf(stderr, "\n");
		va_end(args);
	}
	exit(code);
}

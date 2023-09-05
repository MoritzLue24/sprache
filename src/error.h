#ifndef ERROR_H
#define ERROR_H

#include "loc.h"


enum ErrorCode {
	ERROR_NONE,
	ERROR_MEMORY_ALLOCATION,
	ERROR_CLI_USAGE,
	ERROR_EOF_REACHED,
	ERROR_TOKEN_INVALID,
	ERROR_SYNTAX_INVALID,
    ERROR_NODE_INVALID,
};


/* Writes the error to `stderr`,
 * exits the program.
*/
void
fail(enum ErrorCode code, const char *msg);

/* Writes the error and the occurrence in the source code to `stderr`.
*/
void
fail_spr(enum ErrorCode code, struct Loc loc, const char *msg);

#endif /* ERROR_H */

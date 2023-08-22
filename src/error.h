#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

#define OK (struct Error){ ERROR_NONE, NULL }


struct Error {
	enum ErrorCode {
		ERROR_NONE,
		ERROR_MEMORY_ALLOCATION,
		ERROR_EOF_REACHED,
		ERROR_TOKEN_INVALID,
	} code;
	const char *msg;
};

/* Writes the error to `stderr`,
 * returns the error code.
*/
int
fail(struct Error error);

#endif /* ERROR_H */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>


struct Error {
	enum ErrorType {
		ERROR_NONE,
		ERROR_EOF_REACHED,
		ERROR_TOKEN_INVALID,
	} type;
	const char *msg;
};

/* Writes the error to `stderr`,
 * returns the error code (its just the `enum ErrorType type`).
*/
int
fail(struct Error error);

#endif /* ERROR_H */

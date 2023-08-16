#include "error.h"
#include <stdint.h>


int32_t
fail(struct Error error)
{
	if (error.msg == NULL)
		fprintf(stderr, "ERROR: code %i", error.code);
	else 
		fprintf(stderr, "ERROR: code %i, '%s'", error.code, error.msg);
	return error.code;
}

#include "error.h"
#include <stdint.h>


int32_t
fail(struct Error error)
{
	fprintf(stderr, "%i: '%s'", error.type, error.msg);
	return error.type;
}

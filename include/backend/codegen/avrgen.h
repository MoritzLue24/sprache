#ifndef AVRGEN_H
#define AVRGEN_H

#include "backend/ir/irgen.h"
#include <stdio.h>

void gen_avr(const struct IRFunc* head, FILE* out);

#endif
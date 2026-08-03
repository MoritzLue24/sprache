#ifndef AVRGEN_H
#define AVRGEN_H

#include "backend/ir/irgen.h"
#include <stdio.h>

bool gen_avr(const struct IRInstr* head, FILE* out);

#endif
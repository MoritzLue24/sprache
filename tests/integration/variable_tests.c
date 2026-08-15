/// @brief must come before any include (even tst.h's) so glibc exposes
/// mkdtemp/usleep/kill/PATH_MAX under -std=c11 - see avr_run_fixture.h
#define _POSIX_C_SOURCE 200809L

#include "tst.h"
#include "integration/avr_run_fixture.h"


static void variable_definition_returns_initial_value()
{
    TST_ASSERT_AVR_RETURNS(5, "fn main() { var x = 5; return x; }");
}

static void variable_declaration_then_assignment_returns_assigned_value()
{
    TST_ASSERT_AVR_RETURNS(7, "fn main() { var x; x = 7; return x; }");
}

/// @brief assignment is itself an expression and evaluates to the
/// assigned value
static void assignment_expression_evaluates_to_assigned_value()
{
    TST_ASSERT_AVR_RETURNS(9, "fn main() { var x; return x = 9; }");
}

static void chained_assignment_assigns_all_variables()
{
    TST_ASSERT_AVR_RETURNS(
        10,
        "fn main() { var x; var y; x = y = 5; return x + y; }"
    );
}

static void triple_chained_assignment_assigns_all_variables()
{
    TST_ASSERT_AVR_RETURNS(
        6,
        "fn main() { var x; var y; var z; x = y = z = 2; return x + y + z; }"
    );
}

int main()
{
    TST_RUN(variable_definition_returns_initial_value);
    TST_RUN(variable_declaration_then_assignment_returns_assigned_value);
    TST_RUN(assignment_expression_evaluates_to_assigned_value);
    TST_RUN(chained_assignment_assigns_all_variables);
    TST_RUN(triple_chained_assignment_assigns_all_variables);
    TST_SUMMARY();
    return 0;
}

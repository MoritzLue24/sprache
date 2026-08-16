/// @brief must come before any include (even tst.h's) so glibc exposes
/// mkdtemp/usleep/kill/PATH_MAX under -std=c11 - see avr_run_fixture.h
#define _POSIX_C_SOURCE 200809L

#include "tst.h"
#include "integration/avr_run_fixture.h"


static void call_with_no_parameters()
{
    TST_ASSERT_AVR_RETURNS(
        2,
        "fn two() { return 1 + 1; }"
        "fn main() { return two(); }"
    );
}

static void call_with_two_parameters_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(
        7,
        "fn add(a, b) { return a + b; } "
        "fn main() { return add(3, 4); }"
    );
}

/// @brief distinct operands catch a swapped-argument bug that add() alone
/// couldn't (a + b would still be correct even if a and b were swapped)
static void call_passes_arguments_in_declared_order()
{
    TST_ASSERT_AVR_RETURNS(
        7,
        "fn sub(a, b) { return a - b; } "
        "fn main() { return sub(10, 3); }"
    );
}

static void call_with_three_parameters_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(
        6,
        "fn sum3(a, b, c) { return a + b + c; } "
        "fn main() { return sum3(1, 2, 3); }"
    );
}

static void nested_call_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(
        6,
        "fn add(a, b) { return a + b; } "
        "fn main() { return add(add(1, 2), 3); }"
    );
}

int main()
{
    TST_RUN(call_with_no_parameters);
    TST_RUN(call_with_two_parameters_returns_expected_value);
    TST_RUN(call_passes_arguments_in_declared_order);
    TST_RUN(call_with_three_parameters_returns_expected_value);
    TST_RUN(nested_call_returns_expected_value);
    TST_SUMMARY();
    return 0;
}

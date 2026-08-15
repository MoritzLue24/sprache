/// @brief must come before any include (even tst.h's) so glibc exposes
/// mkdtemp/usleep/kill/PATH_MAX under -std=c11 - see avr_run_fixture.h
#define _POSIX_C_SOURCE 200809L

#include "tst.h"
#include "avr_run_fixture.h"


static void addition_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(5, "fn main() { return 2 + 3; }");
}

static void subtraction_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(6, "fn main() { return 10 - 4; }");
}

static void multiplication_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(42, "fn main() { return 6 * 7; }");
}

static void unary_minus_returns_expected_value()
{
    /*
    -5: two's complement of 5 in 8 bit = 0b1111_1011 = 0xFB = 251
    */
    TST_ASSERT_AVR_RETURNS(0xFB, "fn main() { return -5; }");
}

static void arithmetic_respects_operator_precedence()
{
    /*
    `*` binds tighter than `+`/`-`: 2 + 3 * 4 = 2 + 12 = 14
    */
    TST_ASSERT_AVR_RETURNS(14, "fn main() { return 2 + 3 * 4; }");
}

static void chained_addition_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(10, "fn main() { return 1 + 2 + 3 + 4; }");
}

/// @brief a balanced (not left-deep) 8-leaf sum, so several partial sums
/// have to stay live at once instead of accumulating into one running
/// total - if the compiler ever reused a register that was still holding
/// a live partial sum, this would silently compute the wrong total.
static void nested_balanced_expression_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(36, "fn main() { "
        "var a=1; var b=2; var c=3; var d=4; var e=5; var f=6; var g=7; var h=8; "
        "return ((a + b) + (c + d)) + ((e + f) + (g + h)); }");
}

int main()
{
    TST_RUN(addition_returns_expected_value);
    TST_RUN(subtraction_returns_expected_value);
    TST_RUN(multiplication_returns_expected_value);
    TST_RUN(unary_minus_returns_expected_value);
    TST_RUN(arithmetic_respects_operator_precedence);
    TST_RUN(chained_addition_returns_expected_value);
    TST_RUN(nested_balanced_expression_returns_expected_value);
    TST_SUMMARY();
    return 0;
}

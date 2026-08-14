/// @brief must come before any include (even tst.h's) so glibc exposes
/// mkdtemp/usleep/kill/PATH_MAX under -std=c11 - see avr_run_fixture.h
#define _POSIX_C_SOURCE 200809L

#include "tst.h"
#include "avr_run_fixture.h"


static void bitwise_and_returns_expected_value()
{
    TST_ASSERT_AVR_RETURNS(1, "fn main() { return 5 & 3; }");
}

static void bitwise_or_returns_expected_value()
{
    /*
    5 | 3: 0b00000101 | 0b00000011 = 0b00000111 = 7
    */
    TST_ASSERT_AVR_RETURNS(7, "fn main() { return 5 | 3; }");
}

static void bitwise_xor_returns_expected_value()
{
    /*
    5 ^ 3: 0b00000101 ^ 0b00000011 = 0b00000110 = 6
    */
    TST_ASSERT_AVR_RETURNS(6, "fn main() { return 5 ^ 3; }");
}

static void bitwise_not_returns_complement()
{
    /*
    5: 00000101
    ~5: 0b1111_1010 = 0xFA
    */
    TST_ASSERT_AVR_RETURNS(0xFA, "fn main() { return ~5; }");
}

static void bitwise_mixed_returns_expected_value()
{
    /*
    1 & 9: 0b00000001 & 0b00001001 = 1
    1 ^ 2: 0b00000001 ^ 0b00000010 = 3
    ~3: 0b1111_1100
    2 | ~3: 0b1111_1110 = 0xFE
    */
    TST_ASSERT_AVR_RETURNS(
        0xFE,
        "fn main() { return 2 | ~(1 & 9 ^ 2); }"
    );
}

static void bitwise_or_chained_returns_expected_value()
{
    /*
    2 | 4: 0b00000010 | 0b00000100 = 0b00000110 = 6
    6 | 3: 0b00000110 | 0b00000011 = 0b00000111 = 7
    7 | 20: 0b00000111 | 0b00010100 = 0b00010111 = 0x17
    */
    TST_ASSERT_AVR_RETURNS(
        0x17,
        "fn main() { return 2 | 4 | 3 | 20 ; }"
    );
}

int main()
{
    TST_RUN(bitwise_and_returns_expected_value);
    TST_RUN(bitwise_or_returns_expected_value);
    TST_RUN(bitwise_xor_returns_expected_value);
    TST_RUN(bitwise_not_returns_complement);
    TST_RUN(bitwise_mixed_returns_expected_value);
    TST_RUN(bitwise_or_chained_returns_expected_value);
    TST_SUMMARY();
    return 0;
}

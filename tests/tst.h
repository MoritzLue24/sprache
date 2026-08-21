#ifndef TST_H
#define TST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TST_BOLD "\x1b[1m"
#define TST_GREEN "\x1b[32m"
#define TST_RED "\x1b[31m"
#define TST_RESET "\x1b[0m"

static unsigned int tst_run_cnt = 0;
static unsigned int tst_failed_cnt = 0;
static const char* tst_current_name = NULL;
static bool tst_current_failed;
static bool tst_header_printed = false;

#define TST_ASSERT_EQ(expected, actual) do { \
    __typeof__(actual) _tst_exp = (expected); \
    __typeof__(actual) _tst_act = (actual); \
    if (_tst_exp != _tst_act) { \
        tst_current_failed = true; \
        printf( \
            TST_RED "%*sFAILED: \"%s\" (%s:%d)\n%*sexpected %d, got %d\n" \
            TST_RESET, 2, "", tst_current_name, __FILE__, __LINE__, 4, "", \
            (int)_tst_exp, (int)_tst_act \
        ); \
    } \
} while (0)

#define TST_ASSERT(cond) do { \
    if (!(cond)) { \
        tst_current_failed = true; \
        printf( \
            TST_RED "%*sFAILED: \"%s\" (%s:%d)\n%*scondition failed: %s\n" \
            TST_RESET, 2, "", tst_current_name, __FILE__, __LINE__, 4, "", \
            #cond \
        ); \
    } \
} while (0)

#define TST_ASSERT_STR_CONTAINS(haystack, needle) do { \
    const char* _tst_hay = (haystack); \
    const char* _tst_ndl = (needle); \
    if (strstr(_tst_hay, _tst_ndl) == NULL) { \
        tst_current_failed = true; \
        printf( \
            TST_RED \
            "%*sFAILED: \"%s\" (%s:%d)\n%*sexpected to find \"%s\" in:\n%s\n" \
            TST_RESET, 2, "", tst_current_name, __FILE__, __LINE__, 4, "", \
            _tst_ndl, _tst_hay \
        ); \
    } \
} while (0)

#define TST_RUN(fn) do { \
    if (!tst_header_printed) { \
        printf(TST_BOLD "%s\n" TST_RESET, __FILE__); \
        tst_header_printed = true; \
    } \
    tst_current_name = #fn; \
    tst_current_failed = false; \
    fn(); \
    tst_run_cnt++; \
    if (tst_current_failed) \
        tst_failed_cnt++; \
    else \
        printf("%*ssuccess: \"%s\"\n", 2, "", tst_current_name); \
} while (0)

#define TST_SUMMARY() do { \
    if (tst_failed_cnt != 0) { \
        printf(TST_RED); \
    } \
    else { \
        printf(TST_GREEN); \
    } \
    printf( \
        "%*srun %d, failed %d\n" TST_RESET, 2, "", tst_run_cnt, \
        tst_failed_cnt \
    ); \
    if (tst_failed_cnt != 0) { \
        return 1; \
    } \
} while (0)

#endif

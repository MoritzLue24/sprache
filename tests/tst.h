#ifndef TST_H
#define TST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static unsigned int tst_run_cnt = 0;
static unsigned int tst_failed_cnt = 0;
static const char* tst_current_name = NULL;
static bool tst_current_failed;

#define TST_ASSERT_EQ(expected, actual) do { \
    __typeof__(actual) _tst_exp = (expected); \
    __typeof__(actual) _tst_act = (actual); \
    if (_tst_exp != _tst_act) { \
        tst_current_failed = true; \
        printf("FAILED: %s:%d (%s)\n\texpected %d, got %d\n", \
            __FILE__, __LINE__, tst_current_name, \
            (int)_tst_exp, (int)_tst_act); \
    } \
} while (0)

#define TST_ASSERT(cond) do { \
    if (!(cond)) { \
        tst_current_failed = true; \
        printf("FAILED: %s:%d (%s)\n\tcondition failed: %s\n", \
            __FILE__, __LINE__, tst_current_name, #cond); \
    } \
} while (0)

#define TST_ASSERT_STR_CONTAINS(haystack, needle) do { \
    const char* _tst_hay = (haystack); \
    const char* _tst_ndl = (needle); \
    if (strstr(_tst_hay, _tst_ndl) == NULL) { \
        tst_current_failed = true; \
        printf("FAILED: %s:%d (%s)\n\texpected to find \"%s\" in:\n%s\n", \
            __FILE__, __LINE__, tst_current_name, _tst_ndl, _tst_hay); \
    } \
} while (0)

#define TST_RUN(fn) do { \
    tst_current_name = #fn; \
    tst_current_failed = false; \
    fn(); \
    tst_run_cnt++; \
    if (tst_current_failed) tst_failed_cnt++; \
    else printf("SUCCESS: \"%s\"\n", tst_current_name); \
} while (0)

#define TST_SUMMARY() \
    printf("SUMMARY: run %d, failed %d\n", tst_run_cnt, tst_failed_cnt)

#endif
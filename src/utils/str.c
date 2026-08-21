#include "utils/str.h"
#include "utils/arena.h"
#include <stddef.h>
#include <string.h>
#include <ctype.h>

char* str_upper(char* s)
{
    for (char* p = s; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
    return s;
}

bool str_ends_with(const char* s, const char* suffix)
{
    size_t string_len = strlen(s);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > string_len) {
        return false;
    }
    return strcmp(s + string_len - suffix_len, suffix) == 0;
}

char* str_replace_last(
    struct Arena* a, const char* s, const char* substr, const char* new_substr
) {
    const char* substr_ptr = NULL;
    const char* search = s;

    while (1) {
        const char* next = strstr(search, substr);
        if (next == NULL) {
            break;
        }

        substr_ptr = next;
        search = next + strlen(substr);
    }

    if (substr_ptr == NULL) {
        return NULL;
    }

    size_t len_before = substr_ptr - s;
    size_t len_after = s + strlen(s) - (substr_ptr + strlen(substr));
    char* result = ARENA_CALLOC_LIST(
        a, len_before + strlen(new_substr) + len_after + 1, char
    );

    memcpy(result, s, len_before); // copy before
    memcpy(
        result + len_before + strlen(new_substr),
        s + len_before + strlen(substr), len_after
    ); // copy after
    memcpy(
        result + len_before, new_substr,
        strlen(new_substr)
    ); // copy new substr

    result[len_before + strlen(new_substr) + len_after] = '\0';
    return result;
}

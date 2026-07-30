#include "utils/str.h"
#include "utils/xalloc.h" // xmalloc
#include <string.h> // strstr

char* replace_last(const char* string, const char* substr, const char* new_substr)
{
    const char* substr_ptr = NULL;
    const char* search = string;
    while(1)
    {
        const char *next = strstr(search, substr);
        if (next == NULL)
            break;

        substr_ptr = next;
        search = next + strlen(substr);
    }

    if (substr_ptr == NULL)
        return NULL;

    size_t len_before = substr_ptr - string;
    size_t len_after = string + strlen(string) - (substr_ptr + strlen(substr));

    char* const result = xmalloc(len_before + strlen(new_substr) + len_after + 1);
    memcpy(result, string, len_before); // copy before
    memcpy(result + len_before + strlen(new_substr), string + len_before + strlen(substr), len_after); // copy after
    memcpy(result + len_before, new_substr, strlen(new_substr));    // copy new substr
    result[len_before + strlen(new_substr) + len_after] = '\0';

    return result;
}

bool ends_with(const char* string, const char* suffix)
{
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > string_len)
        return false;

    return strcmp(string + string_len - suffix_len, suffix) == 0;
}

char* substr(const char* string, size_t start, size_t end)
{
    size_t sublen = end - start;
    if (end < start || start >= strlen(string) || end >= strlen(string))
        return NULL;

    if (sublen == 0)
        sublen = 1;

    char* result = xmalloc(sublen + 1);
    strncpy(result, string + start, sublen);
    result[sublen] = '\0';
    return result;
}
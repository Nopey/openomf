#include "utils/allocator.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *omf_malloc_real(size_t size, const char *file, int line) {
    void *ret = malloc(size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "malloc(%zu) failed on %s:%d\n", size, file, line);
    abort();
}

void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
    void *ret = calloc(nmemb, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "calloc(%zu, %zu) failed on %s:%d\n", nmemb, size, file, line);
    abort();
}

void *omf_realloc_real(void *ptr, size_t size, const char *file, int line) {
    void *ret = realloc(ptr, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "realloc(%p, %zu) failed on %s:%d\n", ptr, size, file, line);
    abort();
}

char *omf_strdup_real(char const *c_string, const char *file, int line) {
    char *ret;
#if !defined(HAVE_STD_STRDUP)
    size_t len = strlen(c_string) + 1;
    ret = malloc(len);
    if(ret != NULL)
        memcpy(ret, c_string, len);
#elif defined(_MSC_VER)
    ret = _strdup(c_string);
#else
    ret = strdup(c_string);
#endif
    if(ret != NULL)
        return ret;
    fprintf(stderr, "strdup(%s) failed on %s:%d\n", c_string, file, line);
    abort();
}

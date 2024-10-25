#include "utils/allocator.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#if NDEBUG
#define ALLOCMETA 0
#else
typedef struct {
    size_t alloc_size;
    uint32_t marker;
} metadata_t;
#define ALLOCMETA 1
static uint32_t valid_marker = 0xF3'2097'3F;
#endif


void *omf_malloc_real(size_t size, const char *file, int line) {
#if ALLOCMETA
    metadata_t *meta = malloc(sizeof(metadata_t) + size);
    if(meta == NULL) {
        fprintf(stderr, "malloc(%zu) failed on %s:%d\n", size, file, line);
        abort();
    }
    meta->alloc_size = size;
    meta->marker = valid_marker;
    void *ret = meta + 1;
    memset(ret, 0xCD, size);
#else
    void *ret = malloc(size);
#endif
    if(ret != NULL)
        return ret;
    fprintf(stderr, "malloc(%zu) failed on %s:%d\n", size, file, line);
    abort();
}

void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
#if ALLOCMETA
    // XXX temporarily redirecting all callocs to malloc
    size_t alloc_size = size * nmemb;
    void *ret = omf_malloc_real(alloc_size, file, line);

    // standard calloc zeroes memory.
    // let's see who's relying on this behavior
#ifdef WIN32
#define OMF_PATHSEP "\\"
#else
#define OMF_PATHSEP "/"
#endif
    char const *sources_that_dont_get_zeroed[] = {
        // list of sources (or folders of source) that don't get calloc zeroing behavior
        OMF_PATHSEP "video" OMF_PATHSEP,
        OMF_PATHSEP "resources" OMF_PATHSEP,
        OMF_PATHSEP "formats" OMF_PATHSEP
    };
    bool bZero = true;
    for(int x = 0; x < sizeof sources_that_dont_get_zeroed / sizeof sources_that_dont_get_zeroed[0]; x++)
        bZero &= !strstr(file, sources_that_dont_get_zeroed[x]);
    if(bZero)
        memset(ret, 0, alloc_size);
#else
    void *ret = calloc(nmemb, size);
#endif
    if(ret != NULL)
        return ret;
    fprintf(stderr, "calloc(%zu, %zu) failed on %s:%d\n", nmemb, size, file, line);
    abort();
}

void *omf_realloc_real(void *ptr, size_t size, const char *file, int line) {
#if ALLOCMETA
    if(ptr == NULL)
        return omf_malloc_real(size, file, line);

    metadata_t *meta = ptr;
    meta -= 1;

    if (meta->marker != valid_marker)
    {
        fprintf(stderr, "realloc(%p, %zu) of invalid pointer (marker %u) on %s:%d\n", ptr, size, meta->marker, file, line);
        abort();
    }
    size_t old_size = meta->alloc_size;

    meta = malloc(size + sizeof(metadata_t));
    if (meta == NULL)
    {
        fprintf(stderr, "realloc(%p, %zu) failed on %s:%d\n", ptr, size, file, line);
        abort();
    }
    meta->alloc_size = size;
    meta->marker = valid_marker;
    void *ret = meta + 1;
    if(size >= old_size) {
        memset((char*)ret + old_size, 0xCD, size - old_size);
        memcpy(ret, ptr, old_size);
    } else {
        memcpy(ret, ptr, size);
    }
    omf_free(ptr);
#else
    void *ret = realloc(ptr, size);
#endif
    if(ret != NULL)
        return ret;
    fprintf(stderr, "realloc(%p, %zu) failed on %s:%d\n", ptr, size, file, line);
    abort();
}

char *omf_strdup_real(char const *c_string, const char *file, int line) {
    char *ret;
#if !defined(HAVE_STD_STRDUP) || ALLOCMETA
    size_t len = strlen(c_string) + 1;
    ret = omf_malloc_real(len, file, line);
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

void omf_free_real(void *ptr, char const *file, int line) {
    if(ptr == NULL) {
        return;
    }

#if ALLOCMETA
    metadata_t *meta = ptr;
    meta -= 1;

    if(meta->marker != valid_marker)
    {
        fprintf(stderr, "invalid free(%p) on %s:%d\n", ptr, file, line);
        abort();
    }

    memset(ptr, 0xFE, meta->alloc_size);
    memset(meta, 0xFE, sizeof(metadata_t));

    free(meta);
#endif
}

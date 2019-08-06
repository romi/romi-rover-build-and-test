#include <stdlib.h>

// for log_xxx()
#include <rcom.h>

#include "romi/mem.h"

#define MEM_ALWAYS_ZERO

void *safe_malloc(size_t size)
{
        if (size == 0) {
                log_warn("safe_malloc: size == 0");
                return NULL;
        }
        void *ptr = malloc(size);
        if (ptr == NULL) {
                // Return NULL. Maybe the app can handle it correctly.
                log_err("safe_malloc: out of memory");
                return NULL;
        }
#ifdef MEM_ALWAYS_ZERO
        memset(ptr, 0, size); // zero it anyway.
#endif
        return ptr;
}

void safe_free(void *ptr)
{
        if (ptr == NULL)
                log_warn("safe_free: ptr == NULL");
        else
                free(ptr);
}

void *safe_calloc(size_t nmemb, size_t size)
{
        void *ptr = safe_malloc(nmemb * size);
#ifndef MEM_ALWAYS_ZERO
        if (ptr != NULL) memset(ptr, 0, nmemb * size);
#endif
        return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
        if (size == 0)
                // Not an error
                log_warn("safe_realloc: size == 0"); 

        ptr = realloc(ptr, size);
        if (size > 0 && ptr == NULL) {
                log_err("safe_malloc: out of memory");
                // Return NULL. Maybe the app can handle it correctly.
                return NULL;
        }
        return ptr;
}

char *safe_strdup(const char *s)
{
        if (s == NULL) {
                log_warn("safe_strdup: s == NULL!"); 
                return NULL;
        }
        int len = 0;
        while (s[len] != 0) {
                len++;
                if (len >= 1048576) {
                        log_warn("safe_strdup: strlen(s) > 1MB! Too big."); 
                        return NULL;
                }
        }
        char *t = mem_alloc(len + 1);
        memcpy(t, s, len+1);
        return t;
}

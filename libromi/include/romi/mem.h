
#ifndef _ROMI_MEM_H_
#define _ROMI_MEM_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *safe_malloc(size_t size);
void safe_free(void *ptr);
void *safe_calloc(size_t nmemb, size_t size);
void *safe_realloc(void *ptr, size_t size);

#define new_obj(_type)         ((_type*)safe_malloc(sizeof(_type)))
#define delete_obj(_p)         safe_free(_p)
#define mem_alloc(_size)       safe_malloc(_size)
#define mem_calloc(_n,_size)   safe_calloc(_n,_size)
#define mem_realloc(_p,_size)  safe_realloc(_p,_size)
#define mem_free(_p)           safe_free(_p)
#define new_array(_type,_len)  ((_type*)safe_malloc((_len) * sizeof(_type)))
#define delete_array(_p)       safe_free(_p)

char *safe_strdup(const char *s);
#define mem_strdup(_s)         safe_strdup(_s)

#ifdef __cplusplus
}
#endif

#endif // _ROMI_MEM_H_

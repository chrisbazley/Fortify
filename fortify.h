/* Additional interface for the unmodified Fortify 2.2 distribution. */

#ifndef CB_FORTIFY_H
#define CB_FORTIFY_H

#if !defined(USE_OPTIONAL) && !defined(_Optional)
#define _Optional
#endif

/* Import the original interface without declaring the allocation functions
 * under their exported names. They are redeclared below with appropriately
 * qualified referenced types. */
#define Fortify_Allocate Fortify_Unqualified_Allocate
#define Fortify_malloc Fortify_Unqualified_malloc
#define Fortify_realloc Fortify_Unqualified_realloc
#define Fortify_calloc Fortify_Unqualified_calloc
#define Fortify_free Fortify_Unqualified_free

#include "original/fortify.h"

#undef Fortify_Allocate
#undef Fortify_malloc
#undef Fortify_realloc
#undef Fortify_calloc
#undef Fortify_free

/* This stale disabled-build macro names no function declared by Fortify 2.2. */
#ifdef Fortify_SetMallocFailRate
#undef Fortify_SetMallocFailRate
#endif

#ifdef __cplusplus
extern "C" {
#endif

_Optional void *Fortify_Allocate(size_t size, unsigned char allocator,
                                 const char *file, unsigned long line);

_Optional void *Fortify_malloc(size_t size, const char *file,
                               unsigned long line);
_Optional void *Fortify_realloc(_Optional void *ptr, size_t new_size,
                                const char *file, unsigned long line);
_Optional void *Fortify_calloc(size_t num, size_t size,
                               const char *file, unsigned long line);
void Fortify_free(_Optional void *uptr, const char *file,
                  unsigned long line);

/* Fortify 2.2 documents and defines this name, although its original header
 * declares Fortify_SetFailRate instead.  The wrapper provides both names with
 * identical behaviour. */
int Fortify_SetAllocateFailRate(int percent);

/* Permit at most limit allocation attempts after this call.  ULONG_MAX
 * removes the limit. */
void Fortify_SetNumAllocationsLimit(unsigned long limit);

/* Apply the same fault-injection decision as an intercepted allocation
 * without allocating a Fortify memory block. */
int Fortify_AllowAllocate(const char *file, unsigned long line);

#ifdef __cplusplus
}
#endif

#endif

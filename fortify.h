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
_Optional char *Fortify_strdup(const char *string, const char *file,
                               unsigned long line);
_Optional void *Fortify_aligned_alloc(size_t alignment, size_t size,
                                      const char *file, unsigned long line);
_Optional char *Fortify_strndup(const char *string, size_t max_len,
                                const char *file, unsigned long line);
void Fortify_free_sized(_Optional void *ptr, size_t size, const char *file,
                        unsigned long line);
void Fortify_free_aligned_sized(_Optional void *ptr, size_t alignment,
                                size_t size, const char *file,
                                unsigned long line);
_Optional void *Fortify_reallocarray(_Optional void *ptr, size_t count,
                                     size_t size, const char *file,
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

/* Stream interception is part of the wrapper interface, but it must not be
 * enabled while compiling Fortify itself. Define
 * FORTIFY_NO_STDIO_INTERCEPTION before including this header to use Fortify
 * solely for memory-allocation checking. */
#if defined(FORTIFY) && !defined(__FORTIFY_C__) && \
    !defined(FORTIFY_NO_STDIO_INTERCEPTION)
#include "FortifyIO.h"
#endif

#if defined(FORTIFY) && !defined(__FORTIFY_C__)
#define FORTIFY_INTERCEPTED_MALLOC
#define FORTIFY_INTERCEPTED_CALLOC
#define FORTIFY_INTERCEPTED_REALLOC
#define FORTIFY_INTERCEPTED_FREE
#define FORTIFY_INTERCEPTED_STRDUP
#undef strdup
#define strdup(string) Fortify_strdup(string, __FILE__, __LINE__)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define FORTIFY_INTERCEPTED_ALIGNED_ALLOC
#undef aligned_alloc
#define aligned_alloc(alignment, size) \
    Fortify_aligned_alloc(alignment, size, __FILE__, __LINE__)
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define FORTIFY_INTERCEPTED_STRNDUP
#undef strndup
#define strndup(string, max_len) \
    Fortify_strndup(string, max_len, __FILE__, __LINE__)
#define FORTIFY_INTERCEPTED_FREE_SIZED
#undef free_sized
#define free_sized(ptr, size) \
    Fortify_free_sized(ptr, size, __FILE__, __LINE__)
#define FORTIFY_INTERCEPTED_FREE_ALIGNED_SIZED
#undef free_aligned_sized
#define free_aligned_sized(ptr, alignment, size) \
    Fortify_free_aligned_sized(ptr, alignment, size, __FILE__, __LINE__)
#elif (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L) || \
      (defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L)
#define FORTIFY_INTERCEPTED_STRNDUP
#undef strndup
#define strndup(string, max_len) \
    Fortify_strndup(string, max_len, __FILE__, __LINE__)
#endif
#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 202405L) || \
    (defined(_POSIX_VERSION) && _POSIX_VERSION >= 202405L)
#define FORTIFY_INTERCEPTED_REALLOCARRAY
#undef reallocarray
#define reallocarray(ptr, count, size) \
    Fortify_reallocarray(ptr, count, size, __FILE__, __LINE__)
#endif
#endif

#endif

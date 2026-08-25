/* Additional interface for the unmodified Fortify 2.2 distribution. */

#ifndef CB_FORTIFY_H
#define CB_FORTIFY_H

#include "original/fortify.h"

/* This stale disabled-build macro names no function declared by Fortify 2.2. */
#ifdef Fortify_SetMallocFailRate
#undef Fortify_SetMallocFailRate
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FORTIFY

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

#endif

#ifdef __cplusplus
}
#endif

#endif

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

/* Permit at most limit allocation attempts after this call.  ULONG_MAX
 * removes the limit. */
void Fortify_SetNumAllocationsLimit(unsigned long limit);

#endif

#ifdef __cplusplus
}
#endif

#endif

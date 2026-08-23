#define __FORTIFY_C__
#include "Fortify.h"
#undef __FORTIFY_C__

#include <limits.h>

/* Entry points compiled from the unmodified FORTIFY.CXX. */
void *Fortify22_malloc(size_t size, const char *file, unsigned long line);
void *Fortify22_realloc(void *ptr, size_t size, const char *file,
                        unsigned long line);
void *Fortify22_calloc(size_t count, size_t size, const char *file,
                       unsigned long line);
void Fortify22_free(void *ptr, const char *file, unsigned long line);

/* FORTIFY.CXX defines this name, although FORTIFY.H declares
 * Fortify_SetFailRate instead. */
int Fortify_SetAllocateFailRate(int percent);

static unsigned long allocation_limit = ULONG_MAX;
static unsigned long allocation_count;

static int allocation_permitted(void)
{
    if (allocation_limit == ULONG_MAX)
        return 1;

    if (allocation_count >= allocation_limit)
        return 0;

    ++allocation_count;
    return 1;
}

int Fortify_SetFailRate(int percent)
{
    return Fortify_SetAllocateFailRate(percent);
}

void Fortify_SetNumAllocationsLimit(unsigned long limit)
{
    allocation_limit = limit;
    allocation_count = 0;
}

void *Fortify_malloc(size_t size, const char *file, unsigned long line)
{
    if (!allocation_permitted())
        return NULL;

    return Fortify22_malloc(size, file, line);
}

void *Fortify_calloc(size_t count, size_t size, const char *file,
                     unsigned long line)
{
    if (!allocation_permitted())
        return NULL;

    return Fortify22_calloc(count, size, file, line);
}

void *Fortify_realloc(void *ptr, size_t size, const char *file,
                      unsigned long line)
{
    if (size != 0 && !allocation_permitted())
        return NULL;

    return Fortify22_realloc(ptr, size, file, line);
}

void Fortify_free(void *ptr, const char *file, unsigned long line)
{
    Fortify22_free(ptr, file, line);
}

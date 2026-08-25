#define __FORTIFY_C__
#include "fortify.h"
#undef __FORTIFY_C__

#include <limits.h>
#include <stdio.h>

/* Entry points compiled from the unmodified FORTIFY.CXX. */
void *Fortify22_Allocate(size_t size, unsigned char allocator,
                         const char *file, unsigned long line);
void *Fortify22_malloc(size_t size, const char *file, unsigned long line);
void *Fortify22_realloc(void *ptr, size_t size, const char *file,
                        unsigned long line);
void *Fortify22_calloc(size_t count, size_t size, const char *file,
                       unsigned long line);
void Fortify22_free(void *ptr, const char *file, unsigned long line);
Fortify_OutputFuncPtr Fortify22_SetOutputFunc(Fortify_OutputFuncPtr output);

int Fortify22_SetAllocateFailRate(int percent);

static unsigned long allocation_limit = ULONG_MAX;
static unsigned long allocation_count;
static int allocation_fail_rate;

static void default_output(const char *string)
{
    fprintf(stdout, "%s", string);
    fflush(stdout);
}

static Fortify_OutputFuncPtr output_function = default_output;

#ifdef FORTIFY_WARN_ON_FALSE_FAIL
static const char *allocator_name(unsigned char allocator)
{
    static const char *const names[] = {
        "malloc()", "calloc()", "realloc()", "strdup()", "new", "new[]"
    };

    if (allocator >= sizeof names / sizeof names[0])
        return "unknown allocation";

    return names[allocator];
}
#endif

static void report_false_failure(size_t size, unsigned char allocator,
                                 const char *file, unsigned long line)
{
#ifdef FORTIFY_WARN_ON_FALSE_FAIL
    char buffer[256];

    sprintf(buffer,
            "\nFortify: A \"%s\" of %lu bytes \"false failed\" at %s.%lu\n",
            allocator_name(allocator), (unsigned long)size, file, line);
    output_function(buffer);
#else
    (void)size;
    (void)allocator;
    (void)file;
    (void)line;
#endif
}

static int allocation_permitted(size_t size, unsigned char allocator,
                                const char *file, unsigned long line)
{
    int permitted = 1;

    if (allocation_fail_rate > 0 &&
        rand() % 100 < allocation_fail_rate) {
        permitted = 0;
    } else if (allocation_limit != ULONG_MAX) {
        if (allocation_count >= allocation_limit)
            permitted = 0;
        else
            ++allocation_count;
    }

    if (!permitted)
        report_false_failure(size, allocator, file, line);

    return permitted;
}

int Fortify_SetAllocateFailRate(int percent)
{
    int old = allocation_fail_rate;

    allocation_fail_rate = percent;

    /* The wrapper performs percentage-based fault injection so that
     * Fortify_AllowAllocate and actual allocations share one decision
     * mechanism.  Keep the unmodified implementation's rate at zero to
     * prevent an allocation from being tested a second time. */
    (void)Fortify22_SetAllocateFailRate(0);

    return old;
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

int Fortify_AllowAllocate(const char *file, unsigned long line)
{
    return allocation_permitted(0, Fortify_Allocator_malloc, file, line);
}

Fortify_OutputFuncPtr Fortify_SetOutputFunc(Fortify_OutputFuncPtr output)
{
    Fortify_OutputFuncPtr old = Fortify22_SetOutputFunc(output);

    output_function = output;
    return old;
}

void *Fortify_Allocate(size_t size, unsigned char allocator,
                       const char *file, unsigned long line)
{
    if (!allocation_permitted(size, allocator, file, line))
        return NULL;

    return Fortify22_Allocate(size, allocator, file, line);
}

void *Fortify_malloc(size_t size, const char *file, unsigned long line)
{
    if (!allocation_permitted(size, Fortify_Allocator_malloc, file, line))
        return NULL;

    return Fortify22_malloc(size, file, line);
}

void *Fortify_calloc(size_t count, size_t size, const char *file,
                     unsigned long line)
{
    if (!allocation_permitted(count * size, Fortify_Allocator_calloc,
                              file, line))
        return NULL;

    return Fortify22_calloc(count, size, file, line);
}

void *Fortify_realloc(void *ptr, size_t size, const char *file,
                      unsigned long line)
{
    if (size != 0 &&
        !allocation_permitted(size, Fortify_Allocator_realloc, file, line))
        return NULL;

    return Fortify22_realloc(ptr, size, file, line);
}

void Fortify_free(void *ptr, const char *file, unsigned long line)
{
    Fortify22_free(ptr, file, line);
}

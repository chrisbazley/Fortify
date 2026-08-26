#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "fortify.h"

#ifdef FORTIFY_WARN_ON_FALSE_FAIL
static char output_buffer[256];

static void capture_output(const char *string)
{
    strcpy(output_buffer, string);
}
#endif

int main(void)
{
    _Optional void *first;
    _Optional void *second;
    _Optional char *string;
    int old_rate;

    Fortify_SetNumAllocationsLimit(0);
    string = strdup("test");
    assert(string == NULL);

    Fortify_SetNumAllocationsLimit(ULONG_MAX);
    string = strdup("test");
    assert(string != NULL);
    assert(strcmp(&*string, "test") == 0);
    free(string);

    string = Fortify_strndup("testing", 4, __FILE__, __LINE__);
    assert(string != NULL);
    assert(strcmp(&*string, "test") == 0);
    Fortify_free_sized(string, 5, __FILE__, __LINE__);

    first = Fortify_aligned_alloc(sizeof(double), sizeof(double),
                                  __FILE__, __LINE__);
    assert(first != NULL);
    Fortify_free_aligned_sized(first, sizeof(double), sizeof(double),
                               __FILE__, __LINE__);

    first = Fortify_reallocarray(NULL, 2, 3, __FILE__, __LINE__);
    assert(first != NULL);
    free(first);

    Fortify_SetNumAllocationsLimit(1);
    first = malloc(1);
    second = malloc(1);
    assert(first != NULL);
    assert(second == NULL);
    free(first);

    /* Fortify_AllowAllocate consumes the same allocation-count allowance as
     * an intercepted allocation, without creating a dummy memory block. */
    Fortify_SetNumAllocationsLimit(1);
    assert(Fortify_AllowAllocate(__FILE__, __LINE__));
    assert(malloc(1) == NULL);

    Fortify_SetNumAllocationsLimit(1);
    first = malloc(1);
    assert(first != NULL);
    assert(!Fortify_AllowAllocate(__FILE__, __LINE__));
    free(first);

    Fortify_SetNumAllocationsLimit(ULONG_MAX);
    old_rate = Fortify_SetFailRate(100);
#ifdef FORTIFY_WARN_ON_FALSE_FAIL
    {
        Fortify_OutputFuncPtr old_output = Fortify_SetOutputFunc(capture_output);

        output_buffer[0] = '\0';
        assert(!Fortify_AllowAllocate("allow.c", 123));
        assert(strcmp(output_buffer,
                      "\nFortify: A \"malloc()\" of 0 bytes \"false failed\" "
                      "at allow.c.123\n") == 0);
        (void)Fortify_SetOutputFunc(old_output);
    }
#else
    assert(!Fortify_AllowAllocate(__FILE__, __LINE__));
#endif
    assert(malloc(1) == NULL);
    assert(Fortify_Allocate(1, Fortify_Allocator_malloc,
                            __FILE__, __LINE__) == NULL);
    assert(Fortify_SetFailRate(old_rate) == 100);

    assert(Fortify_AllowAllocate(__FILE__, __LINE__));

    /* The name declared by the original header and the name used by its
     * documentation and implementation control the same setting. */
    assert(Fortify_SetAllocateFailRate(100) == old_rate);
    assert(Fortify_SetFailRate(0) == 100);
    assert(Fortify_SetFailRate(100) == 0);
    assert(Fortify_SetAllocateFailRate(old_rate) == 100);

    assert(Fortify_CheckAllMemory() == 0);
    return EXIT_SUCCESS;
}

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

#include "Fortify.h"

int main(void)
{
    void *first;
    void *second;
    int old_rate;

    Fortify_SetNumAllocationsLimit(1);
    first = malloc(1);
    second = malloc(1);
    assert(first != NULL);
    assert(second == NULL);
    free(first);

    Fortify_SetNumAllocationsLimit(ULONG_MAX);
    old_rate = Fortify_SetFailRate(100);
    assert(malloc(1) == NULL);
    assert(Fortify_SetFailRate(old_rate) == 100);

    assert(Fortify_CheckAllMemory() == 0);
    return EXIT_SUCCESS;
}

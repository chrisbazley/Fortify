#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify.h"

int main(void)
{
    _Optional FILE *stream = tmpfile();
    static const unsigned char data[] = {1, 2, 3, 4};
    unsigned char buffer[sizeof data];

    assert(stream != NULL);
    assert(fwrite(data, sizeof data[0], sizeof data, &*stream) == sizeof data);
    rewind(&*stream);

    Fortify_SetNumAllocationsLimit(0);
    assert(fread(buffer, sizeof buffer[0], sizeof buffer, &*stream) == 0);
    Fortify_SetNumAllocationsLimit(ULONG_MAX);

    assert(ferror(&*stream));
    assert(ftell(&*stream) == 0);
    clearerr(&*stream);
    assert(!ferror(&*stream));

    assert(fread(buffer, sizeof buffer[0], sizeof buffer, &*stream) ==
           sizeof buffer);
    assert(fclose(&*stream) == 0);
    assert(Fortify_CheckAllMemory() == 0);
    return EXIT_SUCCESS;
}

# Fortify 2.2 CMake wrapper

This directory builds Simon P. Bullen's Fortify 2.2 as C without changing or
renaming any file from the original distribution.  The seven original files
are kept together in `original/`.

Projects should include:

```c
#include "Fortify.h"
```

When this directory is added with `add_subdirectory` or `FetchContent`, link a
target to `Fortify`. The target supplies the `FORTIFY` definition and
the required include directory transitively:

```cmake
target_link_libraries(MyTests PRIVATE Fortify)
```

The wrapper implements the `Fortify_SetFailRate` declaration in the original
header (the original source defines it under the different name
`Fortify_SetAllocateFailRate`) and provides `Fortify_SetNumAllocationsLimit`,
which is used by Christopher Bazley's test suites. Setting the
allocation-count limit resets its count; `ULONG_MAX` disables that form of
fault injection.

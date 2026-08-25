# Fortify 2.2 CMake wrapper

This directory builds Simon P. Bullen's Fortify 2.2 as C without changing
any file from the original distribution, except to rename the original
header files as lowercase, since that is how they are included in the
original source files.  The seven original files are kept together in
`original/`.

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

The original header declares `Fortify_SetFailRate`, whereas the documentation
and source define `Fortify_SetAllocateFailRate`. The wrapper provides both
names with identical behaviour. It also provides
`Fortify_SetNumAllocationsLimit` and `Fortify_AllowAllocate`, which are used by
Christopher Bazley's test suites. Setting the allocation-count limit resets
its count; `ULONG_MAX` disables that form of fault injection.
`Fortify_AllowAllocate` applies the same percentage and allocation-count fault
injection as an intercepted allocation without creating a dummy Fortify memory
block. When
`FORTIFY_WARN_ON_FALSE_FAIL` is defined, wrapper-injected failures use the
same diagnostic format and output function as the original implementation.

cmake_minimum_required(VERSION 3.10)

project(Fortify VERSION 2.2 LANGUAGES C)

include(CTest)

add_library(Fortify STATIC
    fortify.c
    original/FORTIFY.CXX
)

# FORTIFY.CXX is deliberately compiled as C, as directed by FORTIFY.DOC, but
# is not renamed.  Rename its allocation entry points for the private copy so
# that fortify.c can add allocation-count fault injection without changing the
# original source file.
set_source_files_properties(original/FORTIFY.CXX PROPERTIES
    LANGUAGE C
    COMPILE_DEFINITIONS
        "Fortify_Allocate=Fortify22_Allocate;Fortify_SetAllocateFailRate=Fortify22_SetAllocateFailRate;Fortify_SetOutputFunc=Fortify22_SetOutputFunc;Fortify_malloc=Fortify22_malloc;Fortify_realloc=Fortify22_realloc;Fortify_calloc=Fortify22_calloc;Fortify_free=Fortify22_free"
)

# LANGUAGE C selects the C compiler and C language flags, but GCC and Clang
# still infer C++ from the original file's .CXX suffix unless told otherwise.
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set_property(SOURCE original/FORTIFY.CXX APPEND_STRING PROPERTY
        COMPILE_FLAGS " -x c"
    )
endif()

target_compile_definitions(Fortify
    PRIVATE FORTIFY
    INTERFACE FORTIFY
)

target_include_directories(Fortify
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(Fortify PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    DEBUG_POSTFIX dbg
)

if(BUILD_TESTING)
    add_executable(FortifyTest tests/FortifyTest.c)
    target_link_libraries(FortifyTest PRIVATE Fortify)
    add_test(NAME FortifyTest COMMAND FortifyTest)
endif()

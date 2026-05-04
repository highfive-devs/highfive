if(TARGET HighFiveWarnings)
    # Allow multiple `include(HighFiveWarnings)`, which would
    # attempt to redefine `HighFiveWarnings` and fail without
    # this check.
    return()
endif()

add_library(HighFiveWarnings INTERFACE)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "GNU"
   OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    target_compile_options(HighFiveWarnings
        INTERFACE
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wunused
            -Woverloaded-virtual
            -Wformat=2
            -Wconversion
            -Wsign-conversion
    )

    # Clang-Cl frontend for Microsoft Compiler does not digest -Wall because it
    # enables some obscure warnings about missing c++98 compatibility. Better is
    # to use Microsoft-style /W4 flag:
    if(NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
        target_compile_options(HighFiveWarnings
            INTERFACE
                -Wall
        )
    else()
        target_compile_options(HighFiveWarnings
            INTERFACE
                /W4
        )
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(HighFiveWarnings
        INTERFACE
            -Wpedantic
            -Wcast-align
            -Wdouble-promotion
    )

    target_compile_options(HighFiveWarnings
        INTERFACE
            -ftemplate-backtrace-limit=0
    )

    if(HIGHFIVE_HAS_WERROR)
        target_compile_options(HighFiveWarnings
            INTERFACE
                -Werror
                -Wno-error=deprecated-declarations
        )
    endif()
endif()

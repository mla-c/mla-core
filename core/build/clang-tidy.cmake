# clang-tidy.cmake
#
# Reusable module for enabling clang-tidy static analysis on a CMake target.
#
# Usage (in any mla-c project):
#
#   include(path/to/core/build/clang-tidy.cmake)
#   mla_enable_clang_tidy(<target>)
#
# The .clang-tidy configuration file at the repository root is picked up
# automatically by clang-tidy when it processes files in that tree.
#
# Control whether analysis runs by setting MLA_ENABLE_CLANG_TIDY (default ON).
# Pass -DMLA_ENABLE_CLANG_TIDY=OFF to cmake to skip the analysis.

option(MLA_ENABLE_CLANG_TIDY "Run clang-tidy static analysis during compilation" ON)

# Locate a usable clang-tidy binary.
if(MLA_ENABLE_CLANG_TIDY)
    find_program(MLA_CLANG_TIDY_EXE
        NAMES
            clang-tidy-18
            clang-tidy-17
            clang-tidy-16
            clang-tidy
        DOC "Path to the clang-tidy executable"
    )

    if(MLA_CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${MLA_CLANG_TIDY_EXE}")
    else()
        message(FATAL_ERROR "clang-tidy not found – static analysis cannot run. "
                            "Install clang-tidy or pass -DMLA_ENABLE_CLANG_TIDY=OFF to disable this check.")
    endif()
endif()

# mla_enable_clang_tidy(<target>)
#
# Attaches clang-tidy to <target> so that every translation unit is analysed
# as part of the normal build.  Has no effect when MLA_ENABLE_CLANG_TIDY is
# OFF or when clang-tidy could not be found.
function(mla_enable_clang_tidy target)
    if(NOT MLA_ENABLE_CLANG_TIDY)
        return()
    endif()

    if(NOT MLA_CLANG_TIDY_EXE)
        return()
    endif()

    set_target_properties(${target} PROPERTIES
        CXX_CLANG_TIDY "${MLA_CLANG_TIDY_EXE}"
    )

    message(STATUS "clang-tidy enabled for target '${target}'")
endfunction()

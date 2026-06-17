# clang-tidy.cmake
#
# Reusable module for enabling clang-tidy static analysis on a CMake target.
#
# Usage (in any mla-c project):
#
#   include(path/to/core/build/clang-tidy.cmake)
#   mla_enable_clang_tidy(<target>)
#
# The shared .clang-tidy configuration file lives next to this module under
# lib/base-lib/build/.clang-tidy. Since that location is not a parent of most
# source files in this repository, pass it explicitly to clang-tidy.
#
# Control whether analysis runs by setting MLA_ENABLE_CLANG_TIDY (default ON).
# Pass -DMLA_ENABLE_CLANG_TIDY=OFF to cmake to skip the analysis.

option(MLA_ENABLE_CLANG_TIDY "Run clang-tidy static analysis during compilation" ON)

set(MLA_CLANG_TIDY_CONFIG_FILE
        "${CMAKE_CURRENT_LIST_DIR}/.clang-tidy"
        CACHE FILEPATH
        "Path to the .clang-tidy configuration file used by clang-tidy"
)

# Locate a usable clang-tidy binary.
if(MLA_ENABLE_CLANG_TIDY)
    if(EXISTS "${MLA_CLANG_TIDY_CONFIG_FILE}")
        file(TO_CMAKE_PATH "${MLA_CLANG_TIDY_CONFIG_FILE}" MLA_CLANG_TIDY_CONFIG_FILE_NORMALIZED)
        message(STATUS "clang-tidy config found: ${MLA_CLANG_TIDY_CONFIG_FILE_NORMALIZED}")
    else()
        message(FATAL_ERROR
                "clang-tidy config file not found: ${MLA_CLANG_TIDY_CONFIG_FILE}. "
                "Pass -DMLA_CLANG_TIDY_CONFIG_FILE=<path> to override the location.")
    endif()

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
        message(FATAL_ERROR
                "clang-tidy not found – static analysis cannot run. "
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

    # ------------------------------------------------------------------------------
    # Base command passed to CMAKE_<LANG>_CLANG_TIDY
    # ------------------------------------------------------------------------------
    set(clang_tidy_command
            "${MLA_CLANG_TIDY_EXE}"
            "--config-file=${MLA_CLANG_TIDY_CONFIG_FILE_NORMALIZED}"
    )

    # ------------------------------------------------------------------------------
    # On Windows (using MSVC STL headers), Clang needs MSVC compatibility flags to
    # avoid parse errors like:
    #   "statement not allowed in constexpr function" in <ratio>
    #
    # These arguments have no effect on Linux/macOS and are ignored by MSVC builds.
    # ------------------------------------------------------------------------------
    if(WIN32)
        list(APPEND clang_tidy_command
                "--extra-arg=-fms-compatibility"
                "--extra-arg=-fdelayed-template-parsing"
        )
    endif()

    # ------------------------------------------------------------------------------
    # Apply clang-tidy to target
    # ------------------------------------------------------------------------------
    set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${clang_tidy_command}"
    )

    message(STATUS
            "clang-tidy enabled for target '${target}' using config '${MLA_CLANG_TIDY_CONFIG_FILE_NORMALIZED}'"
    )
endfunction()
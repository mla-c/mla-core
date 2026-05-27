cmake_minimum_required(VERSION 3.16)
project(mla-core-tests CXX)

# ---------------------------------------------------------------------------
# Toolchain and compiler configuration
# ---------------------------------------------------------------------------
# Apply the shared mla-c language standards (C++11 / C11).
include(${CMAKE_CURRENT_SOURCE_DIR}/core/build/default-toolchain.cmake)

# Apply compiler-specific warning flags (-Wall -Wextra -Wpedantic -Werror …).
include(${CMAKE_CURRENT_SOURCE_DIR}/core/build/default-compile-config.cmake)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
option(MLA_ENABLE_NETWORK
    "Include HTTP/WebSocket/URL/network sources and run network tests" OFF)
option(MLA_ENABLE_EXTERNAL_TASK
    "Include external-task source and run external-task tests" OFF)
option(MLA_ENABLE_FILE_SYSTEM
    "Include file-system source and run file-system tests" OFF)

# MLA_ENABLE_CLANG_TIDY is declared inside clang-tidy.cmake (default ON).

# ---------------------------------------------------------------------------
# clang-tidy static analysis
# ---------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/core/build/clang-tidy.cmake)

# ---------------------------------------------------------------------------
# Required third-party libraries
# ---------------------------------------------------------------------------
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

# ---------------------------------------------------------------------------
# Source file collection
# ---------------------------------------------------------------------------

# Core library sources – recursively collected, then optional groups removed.
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "core/*.cpp")

# Always exclude platform-specific or optional groups from the base list.
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/poc/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/external_task/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/ui/display/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/http/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/url/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/network/.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/rpc/mla_rpc_http.*\\.cpp$")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/ui/web/.*\\.cpp$")

# Optional: network sources (HTTP client/server, WebSocket, URL, RPC-HTTP, web UI)
if(MLA_ENABLE_NETWORK)
    file(GLOB NETWORK_SOURCES CONFIGURE_DEPENDS
        "core/http/*.cpp"
        "core/url/*.cpp"
        "core/network/*.cpp"
    )
    list(APPEND NETWORK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/rpc/mla_rpc_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/rpc/mla_rpc_http_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/ui/web/mla_ui_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/ui/web/mla_ui_web_remote_surface.cpp
    )
endif()

# Optional: external-task source
if(MLA_ENABLE_EXTERNAL_TASK)
    set(EXTERNAL_TASK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/external_task/mla_external_task.cpp
    )
endif()

# Optional: file-system source
if(MLA_ENABLE_FILE_SYSTEM)
    set(FILE_SYSTEM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/core/filesystem/mla_file_system.cpp
    )
endif()

# Test-support library sources
file(GLOB_RECURSE TEST_SUPPORT_SOURCES CONFIGURE_DEPENDS "core-test-support/*.cpp")

# ---------------------------------------------------------------------------
# Test executable
# ---------------------------------------------------------------------------
add_executable(mla_tests
    core-test/test_main.cpp
    ${CORE_SOURCES}
    ${NETWORK_SOURCES}
    ${EXTERNAL_TASK_SOURCES}
    ${FILE_SYSTEM_SOURCES}
    ${TEST_SUPPORT_SOURCES}
)

# Use the repository root as the base include directory so that all project
# headers are reachable as  core/..., core-platform/..., etc.
target_include_directories(mla_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Propagate feature-flag defines to match the enabled source sets.
if(NOT MLA_ENABLE_NETWORK)
    target_compile_definitions(mla_tests PRIVATE mla_test_disable_network=1)
endif()
if(NOT MLA_ENABLE_EXTERNAL_TASK)
    target_compile_definitions(mla_tests PRIVATE mla_test_disable_external_task=1)
endif()
if(NOT MLA_ENABLE_FILE_SYSTEM)
    target_compile_definitions(mla_tests PRIVATE mla_test_disable_file_system=1)
endif()

target_link_libraries(mla_tests PRIVATE
    Threads::Threads
    ZLIB::ZLIB
)

# ---------------------------------------------------------------------------
# clang-tidy analysis
# ---------------------------------------------------------------------------
mla_enable_clang_tidy(mla_tests)

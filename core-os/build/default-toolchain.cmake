
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)


if (MLA_EMSDK_PATH)
    message("Found EMSDK PATH: ${MLA_EMSDK_PATH}")
    # If MLA_EMSDK_PATH is defined, we assume we are building for WebAssembly
    # To Install the Emscripten SDK, follow the instructions at https://emscripten.org/docs/getting_started/downloads.html
    # This is all install an set the path the compile for WebAssembly will be found by the Emscripten SDK
    # Set the CMake toolchain file for Emscripten
    # Because we are already in the toolchain file we include the Emscripten platform file directly
    include(${MLA_EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake)

    message("Configured Emscripten SDK")

endif()



if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(mla_debug_build=1)
    message("Debug build enabled")
endif()

# Track memory allocations in benchmarks
add_compile_definitions(mla_memory_benchmark=1)
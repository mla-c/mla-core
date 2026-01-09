
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

if (MLA_WASM_STANDALONE)
    # Fix: Tell CMake we are cross-compiling to a generic system (WASM), not Windows
    # This prevents CMake from adding flags like -femit-implib or --major-image-version
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR wasm32)
    set(CMAKE_CROSSCOMPILING TRUE)
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

    message(STATUS "Configuring Toolchain for Standalone WASM (Generic System)")
endif()

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

if (MLA_EMSDK_PATH)

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

    # Set Option for smallest possible output
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMALLOC=emmalloc -sALLOW_MEMORY_GROWTH=true -sSTANDALONE_WASM=1 -sNO_FILESYSTEM=1 -sASSERTIONS=0")
    message(STATUS "Configured Emscripten Compiler")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(/W4 /wd4996 /EHsc)

    # Set Options for static linking libc++.dll and libstdc++.dll and libwinpthread-1.dll Even if we dont need it
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    message(STATUS "Configured MSVC Compiler")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

    message(STATUS "Configured Clang Compiler")

    # Zig compiler uses clang but does not support address sanitizer
    if(NOT (CMAKE_CXX_COMPILER MATCHES "zig") AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Enabling Address Sanitizer for Debug Build")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    # Enable all warnings and treat them as errors
    # Disable exceptions to reduce binary size
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -fno-exceptions)

    message(STATUS "Configured GNU Compiler")

    if(LINUX AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Enabling Address Sanitizer for Debug Build")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()

else()
    message(FATAL_ERROR "Unsupported Compiler. ${CMAKE_CXX_COMPILER_ID} is not supported.")
endif()
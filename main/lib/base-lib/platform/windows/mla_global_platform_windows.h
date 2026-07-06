#ifndef MLA_GLOBAL_PLATFORM_WINDOWS_H
#define MLA_GLOBAL_PLATFORM_WINDOWS_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <windows.h>
#include <conio.h>
#include <stdio.h>

#if mla_use_fast_float == 1

// Using fast-float library for optimized float parsing
// https://github.com/fastfloat/fast_float
// Is found about 40% faster than standard library
// the code size is growing by about 15KB

#include "../generic/mla_global_platform_fast_float.h"

#define mla_platform_strtod mla_private_fast_float_strtod
#define mla_platform_strtoll mla_private_fast_float_strtoll
#define mla_platform_strtoull mla_private_fast_float_strtoull

#else

#define mla_platform_strtod mla_private_generic_strtod
#define mla_platform_strtoll mla_private_generic_strtoll
#define mla_platform_strtoull mla_private_generic_strtoull

#endif



void mla_private_windows_sleep(mla_uint32_t milliseconds) {

    Sleep(milliseconds);

}

mla_uint64_t mla_private_windows_system_time_ms() {

    return GetTickCount64();

}

mla_size_t mla_private_windows_std_read(mla_char_t* buffer, mla_size_t size) {

    mla_size_t count = 0;

    // Raw byte pump: read whatever is currently buffered without echoing or
    // translating. Line editing (echo, cursor movement, history, autocomplete)
    // is handled by the CLI line editor in mla_cli_app.cpp, which needs the raw
    // bytes. Special keys (arrows, Home/End, ...) arrive as a two-byte sequence
    // (a 0x00 / 0xE0 prefix followed by a scan code); both bytes are queued by
    // the console, so consecutive _kbhit()/_getch() calls deliver them in order.
    while (_kbhit() == TRUE && count < size - 1) {
        int ch = _getch(); // <- no echo
        buffer[count++] = mla_s_cast<mla_char_t>(ch);
    }

    buffer[count] = '\0';
    return count;

}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access {
    mla_private_generic_memcpy,
        mla_private_generic_memset,
        mla_private_generic_memcmp,
        mla_private_generic_memmove,
        mla_private_generic_strlen,
        mla_private_generic_strstr,
        mla_private_generic_malloc,
        mla_private_generic_free,
        mla_private_generic_on_malloc_failure,
        mla_private_generic_print,
        mla_private_windows_std_read,
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
        mla_private_windows_sleep,
        mla_private_windows_system_time_ms
    };

void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif

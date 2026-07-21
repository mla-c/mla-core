#ifndef MLA_GLOBAL_PLATFORM_WINDOWS_H
#define MLA_GLOBAL_PLATFORM_WINDOWS_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <windows.h>
#include <dbghelp.h>
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

mla_size_t mla_private_windows_get_stack_trace(mla_char_t* buffer, mla_size_t buffer_size) {

    if (buffer == nullptr || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';

    constexpr USHORT max_frames = 64;
    void* frames[max_frames];
    const USHORT frame_count = CaptureStackBackTrace(1, max_frames, frames, nullptr);
    if (frame_count == 0) {
        return 0;
    }

    // DbgHelp functions are not thread-safe. Keep initialization and all
    // symbol lookups under the same process-wide lock.
    static SRWLOCK symbol_lock = SRWLOCK_INIT;
    static bool symbol_initialization_attempted = false;
    static bool symbols_available = false;

    AcquireSRWLockExclusive(&symbol_lock);

    const HANDLE process = GetCurrentProcess();
    if (!symbol_initialization_attempted) {
        symbol_initialization_attempted = true;
        SymSetOptions(SymGetOptions() | SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME);
        symbols_available = SymInitialize(process, nullptr, TRUE) == TRUE;

        // SymInitialize fails when another part of the process has already
        // initialized DbgHelp. Symbol lookup can still be used in that case.
        if (!symbols_available && GetLastError() == ERROR_INVALID_PARAMETER) {
            symbols_available = true;
        }
    }

    mla_size_t written = 0;
    for (USHORT frame_index = 0; frame_index < frame_count && written < buffer_size - 1; ++frame_index) {
        const DWORD64 address = mla_r_cast<DWORD64>(frames[frame_index]);
        DWORD64 displacement = 0;

        struct mla_private_windows_symbol_storage_t {
            SYMBOL_INFO symbol;
            mla_char_t name[MAX_SYM_NAME];
        } symbol_storage{};

        symbol_storage.symbol.SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol_storage.symbol.MaxNameLen = MAX_SYM_NAME;

        mla_char_t line[MAX_SYM_NAME + 64];
        int line_length = 0;
        if (symbols_available &&
            SymFromAddr(process, address, &displacement, &symbol_storage.symbol) == TRUE) {
            line_length = snprintf(
                line,
                sizeof(line),
                "#%u %s + 0x%llx\n",
                mla_s_cast<unsigned int>(frame_index),
                symbol_storage.symbol.Name,
                mla_s_cast<unsigned long long>(displacement));
        } else {
            line_length = snprintf(
                line,
                sizeof(line),
                "#%u 0x%llx\n",
                mla_s_cast<unsigned int>(frame_index),
                mla_s_cast<unsigned long long>(address));
        }

        if (line_length <= 0) {
            continue;
        }

        mla_size_t copy_length = mla_s_cast<mla_size_t>(line_length);
        const mla_size_t remaining = buffer_size - 1 - written;
        if (copy_length > remaining) {
            copy_length = remaining;
        }

        mla_private_generic_memcpy(buffer + written, line, copy_length);
        written += copy_length;
        buffer[written] = '\0';
    }

    ReleaseSRWLockExclusive(&symbol_lock);
    return written;
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
        mla_private_windows_system_time_ms,
        mla_private_windows_get_stack_trace
    };

void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif

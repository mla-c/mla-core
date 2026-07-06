//
// Created by chris on 8/10/2025.
//

#ifndef MLA_GLOBAL_PLATFORM_LINUX_H
#define MLA_GLOBAL_PLATFORM_LINUX_H

#include "../../core/mla_data_types.h"
#include "../generic/mla_global_platform_generic.h"
#include "../../core/lifecycle/mla_lifecycle_events.h"

#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
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


void mla_private_linux_sleep(mla_uint32_t milliseconds) {

    usleep(milliseconds * 1000); // Convert milliseconds to microseconds
}

mla_uint64_t mla_private_linux_system_time_ms() {

    struct timespec ts;
    // Uses a faster, less precise clock source usually sufficient for millisecond-level timing
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (mla_uint64_t)ts.tv_sec * 1000 + (mla_uint64_t)ts.tv_nsec / 1000000;

}

mla_size_t mla_private_linux_std_read(mla_char_t* buffer, mla_size_t size) {
    struct termios oldt;
    struct termios newt;
    int oldf;
    mla_size_t count = 0;
    int ch;

    // Save terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo, we'll echo manually
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Make stdin non-blocking
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    // Read whatever is available
    while (count < size - 1) {
        ch = getchar();
        if (ch == EOF) { break; }

        if (ch == '\n' || ch == '\r') {
            putchar('\n');  // manual echo for newline
            buffer[count++] = '\n';
            break;
        } else {
            putchar(ch);     // echo character to screen
            buffer[count++] = (mla_char_t) ch;
            fflush(stdout);
        }
    }

    buffer[count] = '\0';

    // Restore terminal and flags
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return count;
}

// Initialize low-level memory operations with default implementations
mla_low_level_operations_t g_low_level_access ={
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
        mla_private_linux_std_read,
        mla_platform_strtod,
        mla_platform_strtoll,
        mla_platform_strtoull,
    mla_private_linux_sleep,
    mla_private_linux_system_time_ms
};


void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif

#ifndef MLA_TESTDATATYPES_H
#define MLA_TESTDATATYPES_H

#include "mla_test_global_config.h"

// Platform detection and 32-bit type definitions
#if defined(__LP64__) || defined(_LP64) || defined(__x86_64__) || defined(__aarch64__) || (defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 8)
    // 64-bit Unix/Linux (LP64 model): long is 8 bytes
    #define ___mla_test_internal_int32 int

#elif defined(_WIN32) || defined(_WIN64)
    // Windows (LLP64 model): long is 4 bytes even on 64-bit
    #define ___mla_test_internal_int32 long

#else
    // Fallback for other platforms: use int (safest cross-platform choice)
    #define ___mla_test_internal_int32 int
#endif

#define mla_test_bool_t bool
#define mla_test_int8_t signed char
#define mla_test_uint8_t unsigned char
#define mla_test_int16_t signed short
#define mla_test_uint16_t unsigned short
#define mla_test_int32_t signed ___mla_test_internal_int32
#define mla_test_uint32_t unsigned ___mla_test_internal_int32
#define mla_test_int64_t signed long long
#define mla_test_uint64_t unsigned long long
#define mla_test_float_t float
#define mla_test_double_t double
#define mla_test_char_t char

#define mla_test_pointer_t void*

// Macro to get the filename only
const mla_test_char_t* mla_test_find_filename_from_path(const mla_test_char_t* path);

// Extracts the filename from the full path
// The macro works for both Unix and Windows style paths
#define mla_test_filename_only mla_test_find_filename_from_path(__FILE__)


// Define a print function structure to allow custom print functions
// This is useful for embedded systems where printf may not be available

struct mla_test_print_t {
    void (*print)(const mla_test_char_t* str, mla_test_uint32_t length);
};

extern mla_test_print_t g_test_print;

struct mla_test_mutex_t {
    mla_test_pointer_t (*create_mutex)();
    mla_test_bool_t (*lock_mutex)(mla_test_pointer_t mutex_id);
    void (*unlock_mutex)(mla_test_pointer_t mutex_id);
    void (*destroy_mutex)(mla_test_pointer_t mutex_id);
};

extern mla_test_mutex_t g_test_mutex;

#define mla_test_print(str, len) g_test_print.print(str, len);

struct mla_test_memory_allocator_t {
    mla_test_pointer_t (*malloc)(mla_test_uint32_t size);
    void (*free)(mla_test_pointer_t ptr);
};

extern mla_test_memory_allocator_t g_test_memory_allocator;

#define mla_test_malloc(size) g_test_memory_allocator.malloc(size)
#define mla_test_free(ptr) g_test_memory_allocator.free(ptr)

enum mla_test_output_format_t: mla_test_uint8_t {
    mla_test_output_format_text,
    mla_test_output_format_json
};

/**
 * @brief Prevents the compiler from optimizing away a benchmark result.
 *
 * Writes @p v to a volatile local, forcing the preceding computation to
 * remain visible to the optimizer without measurable overhead.
 *
 * @tparam T Type of the value (deduced automatically).
 * @param  v The value to sink.
 */
template <typename T>
static inline void mla_test_bench_sink(const T& v) {
    volatile T sink = v;
    (void)sink;
}

#endif

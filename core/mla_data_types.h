#ifndef MLA_DATATYPES_H
#define MLA_DATATYPES_H

#include "mla_global_config.h"

/////////////////////////////////////////////////
// Main Datatypes for the MLA framework
////////////////////////////////////////////////

// Platform detection and 32-bit type definitions
#if defined(__LP64__) || defined(_LP64) || defined(__x86_64__) || defined(__aarch64__) || (defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ == 8)
    // 64-bit Unix/Linux (LP64 model): long is 8 bytes
    #define ___mla_internal_int32 int

#elif defined(_WIN32) || defined(_WIN64)
    // Windows (LLP64 model): long is 4 bytes even on 64-bit
    #define ___mla_internal_int32 long

#else
    // Fallback for other platforms: use int (safest cross-platform choice)
    #define ___mla_internal_int32 int
#endif

#define mla_bool_t bool

#define mla_int8_t signed char
#define mla_uint8_t unsigned char
#define mla_int16_t signed short
#define mla_uint16_t unsigned short
#define mla_int32_t signed ___mla_internal_int32
#define mla_uint32_t unsigned ___mla_internal_int32
#define mla_int64_t signed long long
#define mla_uint64_t unsigned long long

#define mla_float_t float
#define mla_double_t double

#define mla_char_t char
#define mla_utf_16_char_t unsigned short
#define mla_utf_32_char_t unsigned ___mla_internal_int32

#define mla_void_t void
#define mla_platform_pointer_t void*

// Shortcut for common types
struct mla_dynamic_data_t {
    union {
        mla_bool_t asBool;
        mla_int8_t asInt8;
        mla_uint8_t asUint8;
        mla_int16_t asInt16;
        mla_uint16_t asUint16;
        mla_int32_t asInt32;
        mla_uint32_t asUint32;
        mla_int64_t asInt64;
        mla_uint64_t asUint64;
        mla_float_t asFloat;
        mla_double_t asDouble;
        mla_platform_pointer_t asPointer;
        mla_char_t asChar;
    };
};

class mla_pointer_t;
struct mla_pointer_memory_manager_t;

class mla_pointer_t {
public:
    // Copy constructor
    mla_pointer_t(const mla_pointer_t& p_Other);

    // Constructor that takes a buffer pointer
    mla_pointer_t(mla_dynamic_data_t payload, mla_pointer_memory_manager_t* memoryManager);

    // Default constructor
    ~mla_pointer_t();

    // Assignment operator
    mla_pointer_t& operator=(const mla_pointer_t& p_Other);

public:
    mla_dynamic_data_t payload;
    mla_pointer_memory_manager_t* memoryManager;
};

mla_platform_pointer_t mla_pointer_get_platform_pointer(const mla_pointer_t& ptr);

template <typename T>
T* mla_pointer_get_data(const mla_pointer_t& ptr) {
    return static_cast<T*>(mla_pointer_get_platform_pointer(ptr));
}

mla_pointer_t mla_pointer_null();
mla_bool_t mla_pointer_is_null(const mla_pointer_t& ptr);
mla_int32_t mla_pointer_ref_count(const mla_pointer_t& ptr);

// Limits for the data types

#define mla_int8_min (-127 - 1)
#define mla_int8_max (127)

#define mla_uint8_min (0)
#define mla_uint8_max (255)

#define mla_int16_min (-32767 - 1)
#define mla_int16_max (32767)

#define mla_uint16_min (0)
#define mla_uint16_max (65535)

#define mla_int32_min (-2147483647 - 1)
#define mla_int32_max (2147483647)

#define mla_uint32_min (0)
#define mla_uint32_max (4294967295U)

#define mla_int64_min (-9223372036854775807L - 1)
#define mla_int64_max (9223372036854775807L)

#define mla_uint64_min (0UL)
#define mla_uint64_max (18446744073709551615ULL)

#define mla_float_min (-3.40282347e+38F)
#define mla_float_max (3.40282347e+38F)

#define mla_double_min (-1.79769313486231570e+308)
#define mla_double_max (1.79769313486231570e+308)

mla_dynamic_data_t mla_dynamic_data_empty();
mla_dynamic_data_t mla_dynamic_data_from_bool(mla_bool_t value);
mla_dynamic_data_t mla_dynamic_data_from_int8(mla_int8_t value);
mla_dynamic_data_t mla_dynamic_data_from_uint8(mla_uint8_t value);
mla_dynamic_data_t mla_dynamic_data_from_int16(mla_int16_t value);
mla_dynamic_data_t mla_dynamic_data_from_uint16(mla_uint16_t value);
mla_dynamic_data_t mla_dynamic_data_from_int32(mla_int32_t value);
mla_dynamic_data_t mla_dynamic_data_from_uint32(mla_uint32_t value);
mla_dynamic_data_t mla_dynamic_data_from_int64(mla_int64_t value);
mla_dynamic_data_t mla_dynamic_data_from_uint64(mla_uint64_t value);
mla_dynamic_data_t mla_dynamic_data_from_float(mla_float_t value);
mla_dynamic_data_t mla_dynamic_data_from_double(mla_double_t value);
mla_dynamic_data_t mla_dynamic_data_from_pointer(mla_platform_pointer_t value);
mla_dynamic_data_t mla_dynamic_data_from_char(mla_char_t value);

typedef void(*mla_pointer_cleanup_hook_t)(mla_platform_pointer_t data, const mla_dynamic_data_t& userData);

#define mla_size_t mla_uint32_t
#define mla_size_min mla_uint32_min
#define mla_size_max mla_uint32_max

#define mla_hash_t mla_size_t

#define mla_byte_t mla_uint8_t
#define mla_byte_min mla_uint8_min
#define mla_byte_max mla_uint8_max

// Atomic types
struct mla_atomic_int32_t {
    volatile mla_int32_t value;
};

// Methode Visibility
#define mla_global extern

// Volatile qualifier for memory operations that require it
#define mla_volatile volatile

/////////////////////////////////////////////////
// Commonly used macros for memory operations and type limits

// Min and max macros for various types
#define mla_min(a, b) ((a) < (b) ? (a) : (b))
#define mla_max(a, b) ((a) > (b) ? (a) : (b))

// Macro for INFINITY
// Platform-specific infinity definitions (no std/C library dependencies)
#if defined(_MSC_VER)
// MSVC - use IEEE 754 bit pattern for float infinity
static const union { mla_uint32_t i; mla_float_t f; } __mla_inf_union = { 0x7F800000U };
#define mla_infinity_pos (__mla_inf_union.f)
#define mla_infinity_neg (-__mla_inf_union.f)
#else
// GCC/Clang - use builtin
#define mla_infinity_pos (__builtin_inff())
#define mla_infinity_neg (-(__builtin_inff()))
#endif


//////////////////////////////////////////////////////
// Loging and Debugging Macros

// Macro to get the filename only
const mla_char_t* mla_find_filename_from_path(const mla_char_t* path);

// Extracts the filename from the full path
// The macro works for both Unix and Windows style paths
#define __FILENAME_ONLY__ mla_find_filename_from_path(__FILE__)

//////////////////////////////////////////////////
/// Low level memory operations and access to printf and other C functions


typedef struct mla_low_level_operations_t {
    // Function pointers for memory operations
    mla_platform_pointer_t (*memcpy)(mla_platform_pointer_t dest, const mla_platform_pointer_t src, mla_size_t size);
    mla_platform_pointer_t (*memset)(mla_platform_pointer_t dest, mla_byte_t value, mla_size_t size);
    mla_int32_t (*memcmp)(const mla_platform_pointer_t dest, const mla_platform_pointer_t src, mla_size_t size);
    mla_platform_pointer_t (*memmove)(mla_platform_pointer_t dest, const mla_platform_pointer_t src, mla_size_t size);

    // Function pointers for string operations
    mla_char_t* (*strcpy)(mla_char_t* dest, const mla_char_t* src);
    mla_size_t (*strlen)(const mla_char_t* str);
    const mla_char_t* (*strstr)(const mla_char_t* str, const mla_char_t* substr);

    // Memory allocation and deallocation functions
    mla_platform_pointer_t (*malloc)(mla_size_t size);
    void (*free)(mla_platform_pointer_t ptr);
    mla_bool_t (*is_gcc_pointer)(const mla_platform_pointer_t ptr);
    void (*on_malloc_failure)(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name);

    // Function pointers for printf and other output functions
    mla_size_t (*print)(const mla_char_t* format, mla_size_t length);
    mla_size_t (*std_read)(mla_char_t* buffer, mla_size_t size);

    // Parsing
    mla_bool_t (*strtod)(const mla_char_t* str, mla_size_t length, mla_double_t* out_value); // String to double
    mla_bool_t (*strtoll)(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value); // String to int64
    mla_bool_t (*strtoull)(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value); // String to uint64

    // Timing
    void (*sleep)(mla_uint32_t milliseconds);
    mla_uint64_t (*system_time_ms)(); // System time in milliseconds

} mla_low_level_operations_t;

// Global variable to hold the low-level memory operations
// This will be initialized in the OS application startup code
mla_global mla_low_level_operations_t g_low_level_access;

// Default memory operations
#define mla_memcpy(dest, src, size) g_low_level_access.memcpy((dest), (src), (size))
#define mla_memset(dest, value, size) g_low_level_access.memset((dest), (value), (size))
#define mla_memcmp(dest, src, size) g_low_level_access.memcmp((dest), (src), (size))
#define mla_memmove(dest, src, size) g_low_level_access.memmove((dest), (src), (size))

// Unsafe C String functions
#define mla_strcpy(dest, src) g_low_level_access.strcpy((dest), (src))
#define mla_strlen(str) g_low_level_access.strlen((str))
#define mla_strstr(str, substr) g_low_level_access.strstr((str), (substr))

// Memory allocation and deallocation

mla_global mla_pointer_memory_manager_t g_default_pointer_memory_manager;

struct mla_pointer_memory_manager_instance_t {
    mla_pointer_memory_manager_t* current;
};

mla_global mla_pointer_memory_manager_instance_t g_pointer_memory_manager_instance;

/**
 * @brief Allocates a raw (unmanaged) block of memory with failure reporting.
 *
 * This is the low-level platform allocator. The caller is fully responsible for
 * the lifetime of the returned pointer and must eventually pass it to
 * `mla_platform_free`. Prefer `mla_malloc` / `mla_malloc_struct` for managed,
 * reference-counted allocations.
 *
 * @param size          Number of bytes to allocate.
 * @param filename      Source file name (supplied by the `mla_platform_malloc` macro via `__FILE__`).
 * @param function_name Calling function name (supplied by the `mla_platform_malloc` macro via `__func__`).
 * @return              Pointer to the newly allocated block, or `nullptr` on failure.
 *                      When allocation fails, `g_low_level_access.on_malloc_failure` is invoked
 *                      before returning `nullptr`.
 */
mla_platform_pointer_t mla_platform_malloc_with_check(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name);

/**
 * @brief Allocates a managed (reference-counted) memory block through the given memory manager.
 *
 * The returned `mla_pointer_t` holds the first reference to the block. When the
 * last reference is released, `cleanup_hook` is called with `cleanup_data`, and
 * the block is freed automatically.
 *
 * This function is the implementation backing all `mla_malloc*` macros. Prefer
 * the macros over calling this function directly so that `__FILE__` / `__func__`
 * context is captured automatically.
 *
 * @param memory_manager Pointer to the `mla_pointer_memory_manager_t` that will
 *                       own the allocation. Must not be `nullptr`.
 * @param size           Number of bytes to allocate for the user payload.
 * @param cleanup_hook   Optional callback invoked just before the memory is freed.
 *                       Pass `nullptr` if no cleanup is required.
 * @param cleanup_data   Arbitrary data forwarded to `cleanup_hook`. Use
 *                       `mla_dynamic_data_empty()` when not needed.
 * @param filename       Source file name (captured by the macro wrapper).
 * @param function_name  Calling function name (captured by the macro wrapper).
 * @return               A `mla_pointer_t` holding the first reference, or a null
 *                       pointer (`mla_pointer_null()`) on allocation failure.
 */
mla_pointer_t mla_malloc_with_check(mla_pointer_memory_manager_t* memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name);

/**
 * @brief Allocates a managed memory block through an explicit memory manager.
 *
 * Forwards to `mla_malloc_with_check`, automatically injecting `__FILE__` and
 * `__func__` for diagnostics.
 *
 * @param memory_manager The `mla_pointer_memory_manager_t*` to use for the allocation.
 * @param size           Number of bytes to allocate.
 * @param cleanup_hook   Callback invoked on deallocation, or `nullptr`.
 * @param cleanup_data   Data forwarded to `cleanup_hook`.
 * @return               A `mla_pointer_t` holding the first reference.
 */
#define mla_malloc_with_manager(memory_manager, size, cleanup_hook, cleanup_data) mla_malloc_with_check(memory_manager, size, cleanup_hook, cleanup_data, __FILE__, __func__)

/**
 * @brief Allocates a managed memory block through the currently active memory manager.
 *
 * Uses `g_pointer_memory_manager_instance.current` as the allocator. This is the
 * primary allocation macro for all managed heap objects.
 *
 * @param size          Number of bytes to allocate.
 * @param cleanup_hook  Callback invoked on deallocation, or `nullptr`.
 * @param cleanup_data  Data forwarded to `cleanup_hook`.
 * @return              A `mla_pointer_t` holding the first reference.
 */
#define mla_malloc(size, cleanup_hook, cleanup_data) mla_malloc_with_manager(g_pointer_memory_manager_instance.current, size, cleanup_hook, cleanup_data)

/**
 * @brief Allocates a managed raw buffer with no cleanup hook.
 *
 * Convenience wrapper around `mla_malloc` for plain byte buffers that require
 * no special teardown logic.
 *
 * @param size Number of bytes to allocate.
 * @return     A `mla_pointer_t` holding the first reference.
 */
#define mla_malloc_buffer(size) mla_malloc(size, nullptr, mla_dynamic_data_empty())

/**
 * @brief Allocates a managed raw buffer through an explicit memory manager, with no cleanup hook.
 *
 * @param memory_manager The `mla_pointer_memory_manager_t*` to use for the allocation.
 * @param size           Number of bytes to allocate.
 * @return               A `mla_pointer_t` holding the first reference.
 */
#define mla_malloc_buffer_with_manager(memory_manager, size) mla_malloc_with_manager(memory_manager, size, nullptr, mla_dynamic_data_empty())

#define mla_malloc_buffer_cleanup_hook(size, clean_up_hook) mla_malloc_with_manager(g_pointer_memory_manager_instance.current, size, clean_up_hook, mla_dynamic_data_empty())

#define mla_malloc_buffer_cleanup_hook_with_manager(memory_manager, size, clean_up_hook) mla_malloc_with_manager(memory_manager, size, clean_up_hook, mla_dynamic_data_empty())

/**
 * @brief Allocates a raw (unmanaged) block of memory.
 *
 * Expands to `mla_platform_malloc_with_check`, injecting the current source file
 * and function name for failure diagnostics. The caller owns the returned pointer
 * and must pass it to `mla_platform_free` when done.
 *
 * Prefer `mla_malloc` / `mla_malloc_struct` for objects that benefit from
 * automatic reference-counted lifetime management.
 *
 * @param size Number of bytes to allocate.
 * @return     Raw `mla_platform_pointer_t` to the allocated block, or `nullptr` on failure.
 */
#define mla_platform_malloc(size) mla_platform_malloc_with_check(size, __FILE__, __func__)

/**
 * @brief Frees a raw (unmanaged) block previously allocated with `mla_platform_malloc`.
 *
 * Do **not** call this on pointers that were allocated via `mla_malloc` /
 * `mla_malloc_struct` — those are freed automatically by `mla_pointer_t` when
 * the last reference is released.
 *
 * @param ptr The `mla_platform_pointer_t` to free. Passing `nullptr` is safe.
 */
#define mla_platform_free(ptr) g_low_level_access.free((ptr))
#define mla_is_gcc_pointer(ptr) g_low_level_access.is_gcc_pointer((ptr))

// Default printf function
#define mla_print(str, len) g_low_level_access.print(str , len)
#define mla_std_read(buffer, size) g_low_level_access.std_read((buffer), (size))

// Parsing functions
#define mla_strtod(str, length, out_value) g_low_level_access.strtod((str), (length), (out_value))
#define mla_strtoll(str, length, out_value) g_low_level_access.strtoll((str), (length), (out_value))
#define mla_strtoull(str, length, out_value) g_low_level_access.strtoull((str), (length), (out_value))

// Sleep function for timing
#define mla_sleep(milliseconds) g_low_level_access.sleep((milliseconds))
#define mla_system_time_ms() g_low_level_access.system_time_ms()


struct mla_pointer_memory_manager_t {

    mla_pointer_t (*malloc)(mla_pointer_memory_manager_t& memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name);
    mla_platform_pointer_t (*get_platform_pointer)(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload);
    void (*incReferences)(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload);
    void (*decReferences)(mla_pointer_memory_manager_t& memory_manager, mla_dynamic_data_t payload);
    mla_int32_t (*get_ref_count)(const mla_pointer_memory_manager_t & memory_manager, mla_dynamic_data_t payload); // return -1 if not supported
};

// Default initializer for structs and classes which is used to initialize items in data structures like arrays or hash maps.
// Is not really a macro but is part of multiple other macros
template < class T >
struct mla_default_initializer {

    static T init() {
        return T();
    }
};

#define mla_default_init(T) mla_default_initializer<T>
#define mla_default_init_ref(T) mla_default_initializer<T*>

//#define mla_offsetof(type, member) ((mla_size_t)((mla_byte_t*)&(((type*)((mla_byte_t*)1))->member) - (mla_byte_t*)1))
#define mla_offsetof(type, member) ((mla_size_t)__builtin_offsetof(type, member))

// Helper to create print a const value
// Usage: mla_print_const("Hello World")
template<mla_size_t N>
mla_size_t mla_print_const(const mla_char_t (&literal)[N]) {
    return mla_print(literal, N-1);  // N includes null terminator
}

template <typename T>
void mla_pointer_default_struct_cleanup(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)userData;

    T* l_Data = reinterpret_cast<T*>(data);

    if (l_Data == nullptr) {
        return;
    }

    *l_Data = T::init();
}

/**
 * @brief Allocates and default-initialises a managed struct through an explicit memory manager.
 *
 * Allocates `sizeof(T)` bytes via `manager` and registers
 * `mla_pointer_default_struct_cleanup<T>` as the cleanup hook, which resets the
 * struct to `T::init()` before the memory is freed.
 *
 * @tparam T       The struct type to allocate. Must provide a static `init()` function.
 * @param  manager The `mla_pointer_memory_manager_t*` to use for the allocation.
 * @return         A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct_with_manager(manager, T) mla_malloc_with_manager(manager, sizeof(T), mla_pointer_default_struct_cleanup<T>, mla_dynamic_data_empty())

/**
 * @brief Allocates and default-initialises a managed struct through the active memory manager.
 *
 * Allocates `sizeof(T)` bytes via `g_pointer_memory_manager_instance.current` and
 * registers `mla_pointer_default_struct_cleanup<T>` as the cleanup hook, which
 * resets the struct to `T::init()` when the last reference is released.
 *
 * Usage:
 * @code
 * mla_pointer_t ptr = mla_malloc_struct(mla_my_struct_t);
 * mla_my_struct_t* data = mla_pointer_get_data<mla_my_struct_t>(ptr);
 * @endcode
 *
 * @tparam T   The struct type to allocate. Must provide a static `init()` function.
 * @return     A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct(T) mla_malloc(sizeof(T), mla_pointer_default_struct_cleanup<T>, mla_dynamic_data_empty())


template <typename T>
using mla_malloc_struct_ex_clean_up_hook_t = void(*)(T& userData);

template <typename T>
void mla_pointer_default_struct_with_extension_cleanup(mla_platform_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)userData;

    T* data_ptr = reinterpret_cast<T*>(data);

    if (data_ptr == nullptr) {
        return;
    }

    T data_resolved = *data_ptr;

    if (userData.asPointer != nullptr) {
        mla_malloc_struct_ex_clean_up_hook_t<T> clean_up_hook = reinterpret_cast<mla_malloc_struct_ex_clean_up_hook_t<T>>(userData.asPointer);
        clean_up_hook(data_resolved);
    }

    data_resolved = T::init();
}

template <typename T>
mla_dynamic_data_t __mla_dynamic_data_from_pointer_cleanup_hook(mla_malloc_struct_ex_clean_up_hook_t<T> clean_up_hook) {
    return mla_dynamic_data_from_pointer(reinterpret_cast<mla_platform_pointer_t>(clean_up_hook));
}


/**
 * @brief Allocates a managed struct with a custom cleanup hook, through an explicit memory manager.
 *
 * Allocates `sizeof(T)` bytes via `manager` and registers
 * `mla_pointer_default_struct_with_extension_cleanup<T>` as the cleanup handler,
 * which calls the user-supplied `clean_up_hook(T&)` before resetting the struct
 * to `T::init()`.
 *
 * @tparam T             The struct type to allocate.
 * @param  manager       The `mla_pointer_memory_manager_t*` to use for the allocation.
 * @param  clean_up_hook A function of type `void(*)(T&)` invoked just before the memory is freed.
 * @return               A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct_cleanup_hook_with_manager(manager, T, clean_up_hook) mla_malloc_with_manager(manager, sizeof(T), mla_pointer_default_struct_with_extension_cleanup<T>, __mla_dynamic_data_from_pointer_cleanup_hook<T>(clean_up_hook))

/**
 * @brief Allocates a managed struct with a custom cleanup hook, through the active memory manager.
 *
 * Convenience wrapper around `mla_malloc_struct_cleanup_hook_with_manager` that uses
 * `g_pointer_memory_manager_instance.current`.
 *
 * @tparam T             The struct type to allocate.
 * @param  clean_up_hook A function of type `void(*)(T&)` invoked just before the memory is freed.
 * @return               A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct_cleanup_hook(T, clean_up_hook) mla_malloc_struct_cleanup_hook_with_manager(g_pointer_memory_manager_instance.current, T, clean_up_hook)

/**
 * @brief Allocates a managed struct and uses `T::clean_up_resource` as the cleanup hook,
 *        through an explicit memory manager.
 *
 * Requires the struct `T` to expose a static member:
 * @code
 * static void clean_up_resource(T& self);
 * @endcode
 *
 * @tparam T       The struct type to allocate. Must expose `T::clean_up_resource`.
 * @param  manager The `mla_pointer_memory_manager_t*` to use for the allocation.
 * @return         A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct_cleanup_extension_with_manager(manager, T) mla_malloc_with_manager(manager, sizeof(T), mla_pointer_default_struct_with_extension_cleanup<T>, __mla_dynamic_data_from_pointer_cleanup_hook<T>(T::clean_up_resource))

/**
 * @brief Allocates a managed struct and uses `T::clean_up_resource` as the cleanup hook,
 *        through the active memory manager.
 *
 * Convenience wrapper around `mla_malloc_struct_cleanup_extension_with_manager` that
 * uses `g_pointer_memory_manager_instance.current`.
 *
 * Requires the struct `T` to expose a static member:
 * @code
 * static void clean_up_resource(T& self);
 * @endcode
 *
 * @tparam T   The struct type to allocate. Must expose `T::clean_up_resource`.
 * @return     A `mla_pointer_t` holding the first reference to the allocated `T`.
 */
#define mla_malloc_struct_cleanup_extension(T) mla_malloc_struct_cleanup_extension_with_manager(g_pointer_memory_manager_instance.current, T)

/**
 * @brief Wraps an externally-owned raw pointer in a non-owning `mla_pointer_t`.
 *
 * The returned `mla_pointer_t` uses the no-op memory manager
 * (`g_noop_pointer_memory_manager`), meaning that reference counting and
 * deallocation are disabled. The caller retains full ownership of `resource`
 * and is responsible for its lifetime.
 *
 * Use this function only when bridging with external APIs that manage their own
 * memory and you need to pass the resource through MLA interfaces that expect
 * a `mla_pointer_t`.
 *
 * @param resource The externally-managed raw pointer to wrap.
 * @return         A non-owning `mla_pointer_t` that holds `resource`.
 */
mla_pointer_t mla_platform_pointer_to_managed_pointer(const mla_platform_pointer_t resource);


#endif

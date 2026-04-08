//
// Platform header for WASM standalone compilation
// This platform uses JavaScript interop for all low-level operations
// eliminating the need for C standard library dependencies
//

#ifndef COREOS_MLA_GLOBAL_PLATFORM_WASM_STANDALONE_H
#define COREOS_MLA_GLOBAL_PLATFORM_WASM_STANDALONE_H

#include "../../core-os/mla_data_types.h"
#include "../../core-os/lifecycle/mla_lifecycle_events.h"

// Declare external functions which must be provided by the wasm host
extern "C" {

    // Memory operations
    __attribute__((import_module("mla"), import_name("external_memcpy")))
    void* external_memcpy(void* dest, const void* src, unsigned int size);
    __attribute__((import_module("mla"), import_name("external_memset")))
    void* external_memset(void* dest, unsigned char value, unsigned int size);
    __attribute__((import_module("mla"), import_name("external_memcmp")))
    int external_memcmp(const void* dest, const void* src, unsigned int size);
    __attribute__((import_module("mla"), import_name("external_memmove")))
    void* external_memmove(void* dest, const void* src, unsigned int size);

    // String operations
    __attribute__((import_module("mla"), import_name("external_strcpy")))
    char* external_strcpy(char* dest, const char* src);
    __attribute__((import_module("mla"), import_name("external_strlen")))
    unsigned int external_strlen(const char* str);
    __attribute__((import_module("mla"), import_name("external_strstr")))
    const char* external_strstr(const char* str, const char* substr);

    // Memory allocation
    __attribute__((import_module("mla"), import_name("external_malloc")))
    void* external_malloc(unsigned int size);
    __attribute__((import_module("mla"), import_name("external_free")))
    void external_free(void* ptr);

    // I/O operations
    __attribute__((import_module("mla"), import_name("external_print")))
    unsigned int external_print(const char* str, unsigned int length);
    __attribute__((import_module("mla"), import_name("external_std_read")))
    unsigned int external_std_read(char* buffer, unsigned int size);

    // Parsing functions
    __attribute__((import_module("mla"), import_name("external_strtod")))
    int external_strtod(const char* str, unsigned int length, double* out_value);
    __attribute__((import_module("mla"), import_name("external_strtoll")))
    int external_strtoll(const char* str, unsigned int length, long long* out_value);
    __attribute__((import_module("mla"), import_name("external_strtoull")))
    int external_strtoull(const char* str, unsigned int length, unsigned long long* out_value);

    // Timing
    __attribute__((import_module("mla"), import_name("external_sleep")))
    void external_sleep(unsigned int milliseconds);

    __attribute__((import_module("mla"), import_name("external_system_time_ms")))
    unsigned long long external_system_time_ms();
}

// In a standalone WASM environment, we typically don't exit, so this can be a no-op.
extern "C" int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle) {
    (void)func;
    (void)arg;
    (void)dso_handle;
    return 0; // Success
}

// WASM standalone implementations using JavaScript functions
mla_pointer_t __wasm_standalone_memcpy(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return external_memcpy(dest, src, size);
}

mla_pointer_t __wasm_standalone_memset(mla_pointer_t dest, mla_byte_t value, mla_size_t size) {
    return external_memset(dest, value, size);
}

mla_int32_t __wasm_standalone_memcmp(const mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return external_memcmp(dest, src, size);
}

mla_pointer_t __wasm_standalone_memmove(mla_pointer_t dest, const mla_pointer_t src, mla_size_t size) {
    return external_memmove(dest, src, size);
}

mla_char_t* __wasm_standalone_strcpy(mla_char_t* dest, const mla_char_t* src) {
    return external_strcpy(dest, src);
}

mla_size_t __wasm_standalone_strlen(const mla_char_t* str) {
    return external_strlen(str);
}

const mla_char_t* __wasm_standalone_strstr(const mla_char_t* str, const mla_char_t* substr) {
    return external_strstr(str, substr);
}

mla_pointer_t __wasm_standalone_malloc(mla_size_t size) {
    return external_malloc(size);
}

void __wasm_standalone_free(mla_pointer_t ptr) {
    external_free(ptr);
}

mla_bool_t __wasm_standalone_is_gcc_pointer(const mla_pointer_t ptr) {
    (void)ptr;
    return false;
}

mla_uint32_t __wasm_standalone_print(const mla_char_t* str, mla_size_t length) {

    return external_print(str, length);
}

void __wasm_standalone_on_malloc_failure(mla_size_t size, const mla_char_t* filename, const mla_char_t* function_name) {
    external_print("Memory allocation failed: ", 26);
    // Convert the size to string
    char size_str[10]; // Enough for 32-bit size
    mla_size_t index = 0;
    mla_size_t temp_size = size;
    if (temp_size == 0) {
        size_str[index++] = '0';
    } else {
        mla_size_t start_index = index;
        while (temp_size > 0) {
            size_str[index++] = '0' + (temp_size % 10);
            temp_size /= 10;
        }
        // Reverse the string
        mla_size_t end_index = index - 1;
        while (start_index < end_index) {
            char temp = size_str[start_index];
            size_str[start_index] = size_str[end_index];
            size_str[end_index] = temp;
            start_index++;
            end_index--;
        }
    }

    external_print(size_str, index);
    external_print(" bytes in ", 10);
    external_print(filename, external_strlen(filename));
    external_print(" (", 2);
    external_print(function_name, external_strlen(function_name));
    external_print(")\n", 2);

}

mla_size_t __wasm_standalone_std_read(mla_char_t* buffer, mla_size_t size) {
    return external_std_read(buffer, size);
}

mla_bool_t __wasm_standalone_strtod(const mla_char_t* str, mla_size_t length, mla_double_t* out_value) {
    return external_strtod(str, length, out_value) != 0;
}

mla_bool_t __wasm_standalone_strtoll(const mla_char_t* str, mla_size_t length, mla_int64_t* out_value) {
    return external_strtoll(str, length, out_value) != 0;
}

mla_bool_t __wasm_standalone_strtoull(const mla_char_t* str, mla_size_t length, mla_uint64_t* out_value) {
    return external_strtoull(str, length, out_value) != 0;
}

void __wasm_standalone_sleep(mla_uint32_t milliseconds) {
    external_sleep(milliseconds);
}

mla_uint64_t __wasm_standalone_system_time_ms() {
    return external_system_time_ms();
}

// Initialize low-level memory operations with WASM standalone implementations
mla_low_level_operations_t g_low_level_access = {
    __wasm_standalone_memcpy,
    __wasm_standalone_memset,
    __wasm_standalone_memcmp,
    __wasm_standalone_memmove,
    __wasm_standalone_strcpy,
    __wasm_standalone_strlen,
    __wasm_standalone_strstr,
    __wasm_standalone_malloc,
    __wasm_standalone_free,
    __wasm_standalone_is_gcc_pointer,
    __wasm_standalone_on_malloc_failure,
    __wasm_standalone_print,
    __wasm_standalone_std_read,
    __wasm_standalone_strtod,
    __wasm_standalone_strtoll,
    __wasm_standalone_strtoull,
    __wasm_standalone_sleep,
    __wasm_standalone_system_time_ms
};

void mla_boot_os_application() {
    // This function can be used to perform any additional bootstrapping
    // required for the OS application, such as initializing logging or other subsystems.

    // Finish boot
    mla_lifecycle_fire_boot_events();
}

#endif // COREOS_MLA_GLOBAL_PLATFORM_WASM_STANDALONE_H

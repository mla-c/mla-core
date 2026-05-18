//
// Created by chris on 5/18/2026.
//

#ifndef MLA_NATIVE_RESOURCE_H
#define MLA_NATIVE_RESOURCE_H

#include "mla_data_types.h"

/**
 * @file mla_native_resource.h
 * @brief Native OS resource management for cross-platform OS integration.
 *
 * ## Overview
 *
 * A **native resource** is a block of memory (or an opaque handle) that is
 * allocated and owned by the host operating system layer — for example a socket
 * descriptor, a file handle, a platform thread, or a driver-level buffer that
 * must be handed directly to an OS API.
 *
 * ## Why the normal memory manager is NOT used here
 *
 * The MLA framework normally routes all heap allocations through the active
 * `mla_pointer_memory_manager_t` instance (`g_pointer_memory_manager_instance`).
 * That instance can be **swapped at runtime** (e.g. in tests) or may apply
 * additional book-keeping (arena tracking, leak detection, etc.).
 *
 * For OS-level resources this redirection is **unsafe**:
 * - The OS API expects a raw, undecorated pointer with no framework header prepended.
 * - A swapped memory manager (e.g. a test double) may intercept, redirect, or
 *   delay the deallocation in ways that corrupt internal OS structures.
 * - Reference-count manipulation on an OS handle can cause double-free or
 *   use-after-free at the kernel boundary.
 *
 * Allocations made through this module therefore bypass the memory manager
 * entirely and call the underlying platform allocator directly
 * (`mla_platform_malloc` / `mla_platform_free`). The resulting storage is still
 * wrapped in a `mla_pointer_t` so it integrates seamlessly with the rest of the
 * framework's ownership model — only the *allocation path* is different.
 *
 * ## Typical usage pattern
 *
 * ```cpp
 * // 1. Allocate native memory for a platform-specific struct.
 * mla_pointer_t resource = mla_malloc_native_resource_struct(MyOsHandle_t);
 *
 * // 2. Get a typed pointer to fill in the OS-level fields.
 * MyOsHandle_t* handle = mla_pointer_get_data<MyOsHandle_t>(resource);
 *
 * // 3. Pass `handle` to the OS API — no framework header is in front of the data.
 * os_register_handle(handle);
 *
 * // 4. When `resource` goes out of scope or its last reference is released,
 * //    MyOsHandle_t::clean_up_resource() is called automatically.
 * ```
 *
 * @note Never use `mla_malloc` / `mla_malloc_struct` for data that will be
 *       passed directly to an OS API. Use the `mla_malloc_native_resource_*`
 *       functions in this module instead.
 */

/**
 * @brief Alias for `mla_dynamic_data_t` representing an external, OS-owned resource.
 *
 * An `mla_native_resource_t` holds the raw data and metadata of a resource whose
 * memory is managed outside the normal MLA memory-manager pipeline (e.g. a
 * platform socket, a file descriptor wrapper, or a driver buffer).
 *
 * Treat this type as an opaque handle — always access it through the
 * `mla_native_resource_*` API rather than reading its internal fields directly.
 */
typedef mla_dynamic_data_t mla_native_resource_t;

/**
 * @brief Creates an empty (invalid) native resource.
 *
 * Use as a sentinel / default value before a real resource has been acquired.
 *
 * @return An `mla_native_resource_t` in its zeroed / invalid state.
 */
mla_native_resource_t mla_native_resource_empty();

/**
 * @brief Callback type invoked when a managed native resource is released.
 *
 * The framework calls this hook exactly once when the last `mla_pointer_t`
 * reference to the resource is dropped. Perform any OS-level teardown here
 * (e.g. closing a socket, releasing a driver handle).
 *
 * @param userData The native resource being destroyed. Do **not** free the
 *                 resource struct itself — the framework handles that.
 */
typedef void(*mla_native_resource_clean_up_hook_t)(const mla_native_resource_t& userData);

/**
 * @brief Wraps an existing external resource in a reference-counted `mla_pointer_t`.
 *
 * Ownership of @p resource is transferred to the returned `mla_pointer_t`.
 * When the last reference is released, @p cleanup_hook is invoked with the
 * resource data so the caller can perform OS-level cleanup.
 *
 * @note The underlying allocation uses `mla_platform_malloc` directly, bypassing
 *       the active `mla_pointer_memory_manager_t`. See file-level documentation
 *       for the rationale.
 *
 * @param resource      The native resource to wrap. Ownership is transferred.
 * @param cleanup_hook  Hook called when the pointer's reference count reaches zero.
 * @return A `mla_pointer_t` that owns @p resource.
 */
mla_pointer_t mla_native_resource_to_managed_pointer(mla_native_resource_t& resource, mla_native_resource_clean_up_hook_t cleanup_hook);

/**
 * @brief Retrieves the `mla_native_resource_t` stored inside a managed pointer.
 *
 * Use this to read back or modify the OS resource data after wrapping it with
 * `mla_native_resource_to_managed_pointer`.
 *
 * @param pointer  A `mla_pointer_t` previously created by
 *                 `mla_native_resource_to_managed_pointer`.
 * @return Pointer to the embedded `mla_native_resource_t`, or `nullptr` if
 *         @p pointer is null or was not created by this module.
 */
mla_native_resource_t* mla_native_resource_from_managed_pointer(const mla_pointer_t& pointer);

/**
 * @brief Callback type invoked when a native resource buffer is released.
 *
 * @param data Raw pointer to the beginning of the buffer that is being freed.
 *             Do NOT call `mla_platform_free` on this pointer — the framework
 *             performs the deallocation after the hook returns.
 */
typedef void(*mla_native_resource_buffer_clean_up_hook_t)(mla_platform_pointer_t data);

/**
 * @brief Allocates a raw buffer for a native OS resource, bypassing the memory manager.
 *
 * The buffer is allocated via `mla_platform_malloc` directly (i.e. the active
 * `mla_pointer_memory_manager_t` is **not** consulted). The result is wrapped in
 * a `mla_pointer_t` so ownership and lifetime are still tracked by the framework.
 *
 * @note Use this function — not `mla_malloc` — whenever the buffer will be passed
 *       to an OS API. Mixing the two allocators at the OS boundary can cause
 *       critical failures.
 *
 * @param size          Number of bytes to allocate.
 * @param clean_up_hook Called with the raw buffer pointer when the last
 *                      `mla_pointer_t` reference is released.
 * @return A `mla_pointer_t` owning the allocated buffer, or a null pointer on
 *         allocation failure.
 */
mla_pointer_t mla_malloc_native_resource_buffer(mla_size_t size, mla_native_resource_buffer_clean_up_hook_t clean_up_hook);

/**
 * @brief Default cleanup dispatcher for structs allocated with
 *        `mla_malloc_native_resource_struct`.
 *
 * Casts the raw @p data pointer to `T*` and calls `T::clean_up_resource(T*)`.
 * The buffer itself is freed by the framework after this function returns.
 *
 * @tparam T  The struct type. Must provide a `static void clean_up_resource(T*)`.
 * @param data Raw pointer to the struct instance being destroyed.
 */
template <typename T>
void mla_malloc_native_resource_struct_cleanup(mla_platform_pointer_t data) {

    T* l_Data = reinterpret_cast<T*>(data);

    if (l_Data == nullptr) {
        return;
    }

    T::clean_up_resource(l_Data);
}

/**
 * @brief Allocates a struct-sized native resource buffer and wires up automatic cleanup.
 *
 * Expands to `mla_malloc_native_resource_buffer(sizeof(T), ...)` with a
 * cleanup hook that calls `T::clean_up_resource(T*)` on release.
 *
 * **The struct type `T` must provide:**
 * ```cpp
 * static void clean_up_resource(T* self);
 * ```
 *
 * **Example:**
 * ```cpp
 * struct MyOsHandle_t {
 *     mla_int32_t fd;
 *
 *     static void clean_up_resource(MyOsHandle_t* self) {
 *         os_close(self->fd);
 *     }
 * };
 *
 * mla_pointer_t handle = mla_malloc_native_resource_struct(MyOsHandle_t);
 * MyOsHandle_t* raw = mla_pointer_get_data<MyOsHandle_t>(handle);
 * raw->fd = os_open("/dev/null");
 * // When handle's last reference is dropped, os_close() is called automatically.
 * ```
 *
 * @param T  The struct type to allocate.
 * @return A `mla_pointer_t` owning a zero-initialised block of `sizeof(T)` bytes.
 */
#define mla_malloc_native_resource_struct(T) mla_malloc_native_resource_buffer(sizeof(T), mla_malloc_native_resource_struct_cleanup<T>)


#endif

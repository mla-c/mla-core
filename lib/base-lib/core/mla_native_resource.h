//
// Created by chris on 5/18/2026.
//

#ifndef MLA_NATIVE_RESOURCE_H
#define MLA_NATIVE_RESOURCE_H

#include "mla_data_types.h"

/**
 * @file mla_native_resource.h
 * @brief Native OS resource handle for cross-platform OS integration.
 *
 * ## Purpose
 *
 * `mla_native_resource_t` is a thin wrapper around `mla_dynamic_data_t` that
 * holds a single raw OS-level value — for example a socket file descriptor
 * (`int`), a Windows `HANDLE` (`void*`), a WASM externref index
 * (`mla_uint32_t`), or any other primitive that the host OS API requires.
 *
 * ## ⚠ Critical: No Memory-Manager Override
 *
 * Unlike every other heap-owned type in the MLA framework, **native resources
 * must NEVER be allocated through `mla_malloc` / `mla_malloc_struct` or any
 * path that routes through the active `mla_pointer_memory_manager_t`**.
 *
 * The memory manager can be replaced at runtime (e.g. in unit-tests via the
 * test-memory allocator).  If an OS handle were wrapped in a managed allocation
 * that goes through the overridden memory manager, the manager could intercept,
 * redirect, or simulate failures of the underlying `mla_platform_malloc` call,
 * which would silently corrupt the OS handle value and cause critical, hard-to-
 * diagnose failures in the communication layer between the framework and the
 * operating system (sockets, file descriptors, thread handles, etc.).
 *
 * The conversion helper `mla_native_resource_to_managed_pointer` deliberately
 * uses the **no-op memory manager** (`g_noop_pointer_memory_manager`) so that
 * the resulting `mla_pointer_t` carries the resource value without any
 * allocator involvement.  The actual OS-level cleanup (e.g. `closesocket`,
 * `close`, `CloseHandle`) is performed exclusively by the `cleanup_hook`
 * callback that the platform layer registers.
 *
 * ## Typical Usage (platform integration layer only)
 *
 * ```cpp
 * // 1. Obtain a raw OS handle (e.g. a POSIX file descriptor)
 * mla_native_resource_t resource = mla_native_resource_empty();
 * resource.asInt32 = ::open("/dev/sensor0", O_RDONLY);
 *
 * // 2. Define a cleanup hook that releases the OS resource
 * static void sensor_fd_cleanup(const mla_native_resource_t& r) {
 *     ::close(r.asInt32);
 * }
 *
 * // 3. Wrap it in a managed pointer for safe lifetime tracking
 * mla_pointer_t ptr = mla_native_resource_to_managed_pointer(resource, sensor_fd_cleanup);
 *
 * // 4. Store the pointer in user-data, a struct field, etc.
 * //    When the last mla_pointer_t copy goes out of scope the cleanup hook
 * //    is called automatically and the fd is closed.
 *
 * // 5. Retrieve the raw resource from an existing managed pointer
 * mla_native_resource_t* raw = mla_native_resource_from_managed_pointer(ptr);
 * if (raw != nullptr) {
 *     int fd = raw->asInt32; // safe to use
 * }
 * ```
 *
 * @note This API is intended **only** for platform-integration code
 *       (e.g. `lib/base-lib/platform/<os>/`).  Application and framework code must
 *       never interact with raw OS handles directly.
 */

/**
 * @brief Opaque handle for a single OS-level resource value.
 *
 * Internally a `mla_dynamic_data_t` union — choose the union member that
 * matches the native type required by the OS API:
 *   - `asInt32`   — POSIX file descriptor, socket fd, etc.
 *   - `asPointer` — Windows `HANDLE`, `SOCKET`, or any `void*` OS handle.
 *   - `asUint32`  — WASM resource index or other unsigned handle.
 */
typedef mla_dynamic_data_t mla_native_resource_t;

/**
 * @brief Returns an empty (zero-initialised) native resource sentinel.
 *
 * Use this to initialise a `mla_native_resource_t` before assigning a real
 * OS handle.  An empty resource holds no valid OS object and must NOT be
 * passed to any OS API.
 *
 * @return A zero-initialised `mla_native_resource_t`.
 */
mla_native_resource_t mla_native_resource_empty();

/**
 * @brief Callback invoked when the managed pointer's reference count reaches
 *        zero and the OS resource must be released.
 *
 * The implementation must perform the platform-specific teardown for the
 * resource stored in @p userData (e.g. `close(userData.asInt32)` on POSIX,
 * `CloseHandle(userData.asPointer)` on Windows).
 *
 * @param userData  The native resource value that was passed to
 *                  `mla_native_resource_to_managed_pointer`.
 */
typedef void(*mla_native_resource_clean_up_hook_t)(const mla_native_resource_t& userData);

/**
 * @brief Wraps a native OS resource in a reference-counted `mla_pointer_t`
 *        **without** routing through the active memory manager.
 *
 * The resulting `mla_pointer_t` uses the no-op memory manager so that no
 * framework allocator is involved.  Ownership and lifetime are controlled
 * solely by the reference count: when the count drops to zero @p cleanup_hook
 * is called to free the OS-level resource.
 *
 * @param resource      The OS handle to wrap.  The value is copied into the
 *                      pointer's payload; the original variable is not modified.
 * @param cleanup_hook  Platform-provided function that releases the OS resource.
 *                      Must not be `nullptr`.
 * @return A `mla_pointer_t` that owns the native resource.
 *
 * @warning Never allocate the underlying storage through `mla_malloc` or any
 *          overridable allocator path — see the file-level note above.
 */
mla_pointer_t mla_native_resource_to_managed_pointer(mla_native_resource_t& resource, mla_native_resource_clean_up_hook_t cleanup_hook);

/**
 * @brief Retrieves the raw `mla_native_resource_t` stored inside a managed
 *        pointer that was created with `mla_native_resource_to_managed_pointer`.
 *
 * @param pointer  The managed pointer previously returned by
 *                 `mla_native_resource_to_managed_pointer`.
 * @return A non-owning pointer to the resource value, or `nullptr` if
 *         @p pointer is null or does not hold a native resource.
 *
 * @note The returned pointer is valid only for the lifetime of @p pointer.
 *       Do NOT store it beyond the scope in which @p pointer is alive.
 */
mla_native_resource_t* mla_native_resource_from_managed_pointer(const mla_pointer_t& pointer);

#endif
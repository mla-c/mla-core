//
// Created by christian on 9/10/25.
//

#include "mla_memory_hook.h"

// Global memory hook for custom memory management
mla_global_memory_hook_list_t g_memory_hook = {
    nullptr,
    nullptr,
    nullptr,
    0,
    {}
};

static mla_platform_pointer_t __mla_memory_malloc_hook(mla_size_t size) {

    for (mla_int8_t i = 0; i < g_memory_hook.hook_count; ++i) {
        if (g_memory_hook.hooks[i].malloc_hook) {
            mla_platform_pointer_t out_ptr = nullptr;
            if (g_memory_hook.hooks[i].malloc_hook(size, &out_ptr)) {
                return out_ptr;
            }
        } else {
            break; // No more hooks to check
        }
    }

    return g_memory_hook.original_malloc(size);
}

static void __mla_memory_free_hook(mla_platform_pointer_t ptr) {

    for (mla_int8_t i = 0; i < g_memory_hook.hook_count; ++i) {
        if (g_memory_hook.hooks[i].malloc_hook) {
            mla_bool_t done = g_memory_hook.hooks[i].free_hook(ptr);
            if (done) {
                return;
            }
        } else {
            break; // No more hooks to check
        }
    }

    g_memory_hook.original_free(ptr);
}

static mla_bool_t __mla_memory_is_gcc_pointer(const mla_platform_pointer_t ptr) {

    for (mla_int8_t i = 0; i < g_memory_hook.hook_count; ++i) {
        if (g_memory_hook.hooks[i].is_gcc_pointer) {
            MLA_MEMORY_POINTER_TYPE type = g_memory_hook.hooks[i].is_gcc_pointer(ptr);
            if (type == MLA_MEMORY_POINTER_TYPE_GCC) {
                return true;
            } else if (type == MLA_MEMORY_POINTER_TYPE_REGULAR) {
                return false;
            }
            // If UNKNOWN, continue to the next hook
        } else {
            break; // No more hooks to check
        }
    }

    return g_memory_hook.original_is_gcc_pointer(ptr);
}


mla_memory_hook_t mla_memory_hook_install(mla_memory_malloc_hook_t malloc_hook, mla_memory_free_hook_t free_hook, mla_memory_is_gcc_pointer_hook_t is_gcc_pointer) {

    mla_int8_t nextFreeSpace = g_memory_hook.hook_count;

    if (nextFreeSpace >= mla_global_config_max_memory_hooks) {
        return { nullptr, nullptr, nullptr }; // No space for new hook
    }

    mla_memory_hook_t hook = {
            malloc_hook,
            free_hook,
            is_gcc_pointer
    };

    g_memory_hook.hooks[nextFreeSpace] = hook;
    g_memory_hook.hook_count++;

    if (nextFreeSpace == 0) {

        g_memory_hook.original_is_gcc_pointer = g_low_level_access.is_gcc_pointer;
        g_memory_hook.original_free = g_low_level_access.free; // Save the original free function
        g_memory_hook.original_malloc = g_low_level_access.malloc; // Save


        // If this is the first hook, we need to initialize the global memory hook manager
        g_low_level_access.malloc = __mla_memory_malloc_hook; // Override the malloc function
        g_low_level_access.free = __mla_memory_free_hook; // Override the free function
        g_low_level_access.is_gcc_pointer = __mla_memory_is_gcc_pointer;
    }

    return hook;

}

mla_bool_t mla_memory_hook_uninstall(const mla_memory_hook_t &hook) {

    for (mla_int8_t i = 0; i < g_memory_hook.hook_count; ++i) {

        if (g_memory_hook.hooks[i].malloc_hook != hook.malloc_hook ||
            g_memory_hook.hooks[i].free_hook != hook.free_hook ||
            g_memory_hook.hooks[i].is_gcc_pointer != hook.is_gcc_pointer) {
            continue;
        }

        // Copy the memory to close the gap
        mla_memmove((&g_memory_hook.hooks) + i, (&g_memory_hook.hooks) + i + 1, (g_memory_hook.hook_count - i - 1) * sizeof(mla_memory_hook_t));

        g_memory_hook.hook_count--; // Decrease the count of hooks

        if (g_memory_hook.hook_count == 0) {
            // If this was the last hook, restore the original malloc and free functions
            g_low_level_access.malloc = g_memory_hook.original_malloc;
            g_low_level_access.free = g_memory_hook.original_free;
            g_low_level_access.is_gcc_pointer = g_memory_hook.original_is_gcc_pointer;
            g_memory_hook.original_malloc = nullptr; // Clear the original malloc function
            g_memory_hook.original_free = nullptr; // Clear the original free function
            g_memory_hook.original_is_gcc_pointer = nullptr; // Clear the original is_gcc_pointer function
            g_memory_hook.hook_count = 0; // Reset the hook count
        }


        return true; // Successfully uninstalled the hook

    }

    return false;

}
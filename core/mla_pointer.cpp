//
// Created by chris on 4/14/2026.
//

#include "mla_data_types.h"

mla_pointer_t mla_pointer_null() {
    return {
        mla_dynamic_data_empty(),
        nullptr
    };
}

mla_bool_t mla_pointer_is_null(const mla_pointer_t& ptr) {
    return ptr.memoryManager == nullptr || ptr.memoryManager->get_platform_pointer(*ptr.memoryManager, ptr.payload) == nullptr;
}

mla_int32_t mla_pointer_ref_count(const mla_pointer_t& ptr) {

    if (ptr.memoryManager == nullptr) {
        return 0;
    }

    return ptr.memoryManager->get_ref_count(*ptr.memoryManager, ptr.payload);

}

mla_platform_pointer_t mla_pointer_get_platform_pointer(const mla_pointer_t& ptr) {

    if (ptr.memoryManager == nullptr) {
        return nullptr;
    }

    return ptr.memoryManager->get_platform_pointer(*ptr.memoryManager, ptr.payload);

}

mla_pointer_t::mla_pointer_t(const mla_pointer_t& p_Other) : payload(p_Other.payload), memoryManager(p_Other.memoryManager) {

    if (this->memoryManager != nullptr && this->memoryManager->incReferences != nullptr) {
        this->memoryManager->incReferences(*this->memoryManager, this->payload);
    }
}

mla_pointer_t::mla_pointer_t(mla_dynamic_data_t payload, mla_pointer_memory_manager_t* memoryManager) : payload(payload), memoryManager(memoryManager) {

    if (this->memoryManager != nullptr && this->memoryManager->incReferences != nullptr) {
        this->memoryManager->incReferences(*this->memoryManager, this->payload);
    }
}

void __mla_pointer_destroy(mla_pointer_memory_manager_t* memoryManager, mla_dynamic_data_t payload) {

    if (memoryManager == nullptr || memoryManager->decReferences == nullptr) {
        return;
    }

    memoryManager->decReferences(*memoryManager, payload);
}


mla_pointer_t::~mla_pointer_t() {
    __mla_pointer_destroy(memoryManager, payload);
    payload.asPointer = nullptr;
    memoryManager = nullptr;

}

mla_pointer_t& mla_pointer_t::operator=(const mla_pointer_t& p_Other) {

    if (this != &p_Other) {
        // Destroy current buffer

        // Because by destroying the current buffer is can happend
        // that the p_Other is Zeroed out.
        // This happends if there are cyclic references. link in a linked list.
        // So we need to backup the buffer pointer before destroying it.
        // It will not destroy the buffer of p_Other, because it is a reference.
        // Only the pointer for holding the reference is destroyed.
        mla_dynamic_data_t backupBuffer = p_Other.payload;
        mla_pointer_memory_manager_t* backupMemoryManager = p_Other.memoryManager;

        __mla_pointer_destroy(memoryManager, payload); // Destroy current buffer

        // Assign new buffer
        memoryManager = backupMemoryManager;
        payload = backupBuffer; // Assign new buffer

        if (this->memoryManager != nullptr && this->memoryManager->incReferences != nullptr) {
            this->memoryManager->incReferences(*this->memoryManager, this->payload);
        }
    }

    return *this;
}

mla_pointer_t mla_malloc_with_check(mla_pointer_memory_manager_t* memory_manager, mla_size_t size, mla_pointer_cleanup_hook_t cleanup_hook, mla_dynamic_data_t cleanup_data, const mla_char_t* filename, const mla_char_t* function_name) {
    if (size == 0) {
        return mla_pointer_null();
    }

    if (memory_manager == nullptr) {
        return mla_pointer_null();
    }

    return memory_manager->malloc(*memory_manager, size, cleanup_hook, cleanup_data, filename, function_name);
}
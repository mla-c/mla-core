//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_LINK_LIST_H
#define COREOS_MLA_LINK_LIST_H

#include "../mla_data_types.h"
#include "mla_buffer.h"


#define mla_list_list_template typename  T, typename TInit = mla_default_init(T)

const mla_size_t MLA_LINK_LIST_DEFAULT_POOL_SIZE = 16;

template < mla_list_list_template >
struct mla_link_list_node_t;

template < mla_list_list_template >
struct mla_link_list_memory_pool_block_t {
    mla_link_list_memory_pool_block_t<T, TInit>* next;
    mla_byte_t* data;
};

template < mla_list_list_template >
struct mla_link_list_memory_pool_t {
    mla_link_list_memory_pool_block_t<T, TInit>* blocks;
    mla_link_list_node_t<T, TInit>* freeList;
    mla_size_t nodeSize;
    mla_size_t blockSize;
};


template < mla_list_list_template >
mla_link_list_memory_pool_t<T, TInit>* __mla_link_list_memory_pool_create(mla_size_t p_NodeSize, mla_size_t p_BlockSize) {
    mla_link_list_memory_pool_t<T, TInit>* pool = static_cast<mla_link_list_memory_pool_t<T, TInit>*>(mla_malloc(sizeof(mla_link_list_memory_pool_t<T, TInit>)));
    if (!pool) {
        return nullptr;
    }
    pool->blocks = nullptr;
    pool->freeList = nullptr;
    pool->nodeSize = p_NodeSize;
    pool->blockSize = p_BlockSize;
    return pool;
}

template < mla_list_list_template >
void __mla_link_list_memory_pool_destroy(mla_link_list_memory_pool_t<T, TInit>* p_Pool) {
    mla_link_list_memory_pool_block_t<T, TInit>* current = p_Pool->blocks;
    while (current) {
        mla_link_list_memory_pool_block_t<T, TInit>* next = current->next;
        mla_free(current->data);
        mla_free(current);
        current = next;
    }
    mla_free(p_Pool);
}

template < mla_list_list_template >
mla_link_list_node_t<T, TInit>* __mla_link_list_memory_pool_alloc(mla_link_list_memory_pool_t<T, TInit>* p_Pool) {
    if (p_Pool->freeList) {
        mla_link_list_node_t<T, TInit>* node = p_Pool->freeList;
        p_Pool->freeList = node->next;
        return node;
    }

    mla_link_list_memory_pool_block_t<T, TInit>* newBlock = static_cast<mla_link_list_memory_pool_block_t<T, TInit>*>(mla_malloc(sizeof(mla_link_list_memory_pool_block_t<T, TInit>)));
    if (!newBlock) {
        return nullptr;
    }

    newBlock->data = static_cast<mla_byte_t*>(mla_malloc(p_Pool->nodeSize * p_Pool->blockSize));
    if (!newBlock->data) {
        mla_free(newBlock);
        return nullptr;
    }

    newBlock->next = p_Pool->blocks;
    p_Pool->blocks = newBlock;

    for (mla_size_t i = 0; i < p_Pool->blockSize; ++i) {
        mla_link_list_node_t<T, TInit>* node = reinterpret_cast<mla_link_list_node_t<T, TInit>*>(newBlock->data + i * p_Pool->nodeSize);
        node->next = p_Pool->freeList;
        p_Pool->freeList = node;
    }

    mla_link_list_node_t<T, TInit>* node = p_Pool->freeList;
    p_Pool->freeList = node->next;
    return node;
}

template < mla_list_list_template >
void __mla_link_list_memory_pool_free(mla_link_list_memory_pool_t<T, TInit>* p_Pool, mla_link_list_node_t<T, TInit>* p_Node) {
    p_Node->next = p_Pool->freeList;
    p_Pool->freeList = p_Node;
}


template < mla_list_list_template >
struct mla_link_list_node_t {
    mla_link_list_node_t<T, TInit>* next; // Pointer to the next node in the list
    mla_link_list_node_t<T, TInit>* prev; // Pointer to the previous node in the list
    T data;
};

template < mla_list_list_template >
struct mla_link_list_data_t {
    mla_link_list_node_t<T, TInit>* head; // Pointer to the first node in the list
    mla_link_list_node_t<T, TInit>* tail; // Pointer to the last node in the list
    mla_link_list_memory_pool_t<T, TInit>* memoryPool;
};

template < mla_list_list_template >
struct mla_link_list_t {

    mla_size_t size; // Number of elements in the list
    mla_link_list_data_t<T, TInit>* data;
    mla_buffer_reference_t dataOwner; // Reference to the data buffer
};

template < mla_list_list_template >
mla_buffer_cleanup_mode __mla_link_list_node_cleanup_hook(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData; // Silences the unused parameter warning

    // Custom cleanup hook for the linked list node
    mla_link_list_node_t<T, TInit>* node = static_cast<mla_link_list_node_t<T, TInit>*>(data);
    if (node) {
        node->data = TInit::init(); // Assign default value to trigger destructor if T is a class

    }

    return CLEAN_UP_NEEDED;
}

template < mla_list_list_template >
mla_buffer_cleanup_mode __mla_link_list_data_cleanup_hook(mla_pointer_t p_Data, mla_callback_userdata userData) {
    (void)userData; // Silences the unused parameter warning

    // Custom cleanup hook for the linked list node
    mla_link_list_data_t<T, TInit>* data = static_cast<mla_link_list_data_t<T, TInit>*>(p_Data);

    if (data->memoryPool) {
        __mla_link_list_memory_pool_destroy(data->memoryPool);
        data->memoryPool = nullptr;
    }

    return CLEAN_UP_NEEDED;
}

template < mla_list_list_template >
inline mla_link_list_data_t<T, TInit>* __mla_link_list_data() {

    // Allocate memory for the linked list data structure
    mla_link_list_data_t<T, TInit>* data = static_cast<mla_link_list_data_t<T, TInit>*>(mla_malloc(sizeof(mla_link_list_data_t<T, TInit>)));

    if (data == nullptr) {
        return nullptr; // Return null if memory allocation fails
    }

    mla_memset(data, 0, sizeof(mla_link_list_data_t<T, TInit>)); // Initialize the data structure
    data->memoryPool = __mla_link_list_memory_pool_create<T, TInit>(sizeof(mla_link_list_node_t<T, TInit>), MLA_LINK_LIST_DEFAULT_POOL_SIZE);
    if (!data->memoryPool) {
        mla_free(data);
        return nullptr;
    }
    return data; // Return the initialized data structure
}

template < mla_list_list_template >
inline mla_link_list_t<T, TInit>  mla_link_list_empty() {

    return {
        0,
        nullptr,
        mla_buffer_reference_noOwner() // Initialize with no data owner
    };

}

template < mla_list_list_template >
inline mla_link_list_t<T, TInit> mla_link_list() {

    mla_link_list_data_t<T, TInit>* data = __mla_link_list_data<T, TInit>();

    if (data == nullptr) {
        return mla_link_list_empty<T, TInit>(); // Return an empty list if memory allocation fails
    }

    mla_link_list_t<T, TInit> list = { 0, data, mla_buffer_reference(data, true, __mla_link_list_data_cleanup_hook<T, TInit>) };
    return list; // Initialize an empty linked list
}

template < mla_list_list_template >
void mla_link_list_destroy(mla_link_list_t<T, TInit>& list) {

    list.size = 0; // Reset the size
    list.dataOwner = mla_buffer_reference_noOwner(); // Clear data owner reference
    list.data = nullptr; // Clear the head pointer
}

template < mla_list_list_template >
mla_bool_t mla_link_list_add(mla_link_list_t<T, TInit>& list, const T& item) {

    if (list.data == nullptr) {
        // If the list is empty, initialize it
        list.data = __mla_link_list_data<T, TInit>();

        if (list.data == nullptr) {
            return false; // Return false if memory allocation fails
        }

        list.dataOwner = mla_buffer_reference(list.data, true, __mla_link_list_data_cleanup_hook<T, TInit>);
    }

    // Create a new node from the memory pool
    mla_link_list_node_t<T, TInit>* newNode = __mla_link_list_memory_pool_alloc(list.data->memoryPool);

    if (newNode == nullptr) {
        return false; // Return false if memory allocation fails
    }

    newNode->data = item; // Set the data
    newNode->prev = list.data->tail; // Set previous pointer to the current tail
    newNode->next = nullptr; // Set next pointer to null

    if (list.data->tail) {
        list.data->tail->next = newNode; // Link the new node to the current tail
    } else {
        list.data->head = newNode; // If the list was empty, set head to the new node
    }
    list.data->tail = newNode; // Update the tail to the new node
    list.size++; // Increment the size of the list
    return true;
}

template < mla_list_list_template >
mla_bool_t mla_link_list_remove(mla_link_list_t<T, TInit>& list, mla_int32_t index) {

    if (list.size == 0) {
        return false;
    }


    if (list.data == nullptr) {
        // If the list is empty, initialize it
        list.data = __mla_link_list_data<T, TInit>();

        if (list.data == nullptr) {
            return false; // Return false if memory allocation fails
        }

        list.dataOwner = mla_buffer_reference(list.data, true, __mla_link_list_data_cleanup_hook<T, TInit>);
    }

    mla_link_list_node_t<T, TInit>* current = list.data->head;
    while (current) {
        if (index == 0) {
            // If the node to remove is the head
            if (current->prev) {
                current->prev->next = current->next; // Link previous node to next node
            } else {
                list.data->head = current->next; // Update head if removing the first node
            }
            if (current->next) {
                current->next->prev = current->prev; // Link next node to previous node
            } else {
                list.data->tail = current->prev; // Update tail if removing the last node
            }

            __mla_link_list_memory_pool_free(list.data->memoryPool, current);

            list.size--; // Decrement the size of the list
            return true; // Return true indicating successful removal
        }
        index--; // Decrement the index
        current = current->next; // Move to the next node
    }

    return false;
}

template < mla_list_list_template >
inline mla_size_t mla_link_list_size(const mla_link_list_t<T, TInit>& list) {
    return list.size; // Return the current size of the linked list
}

template < mla_list_list_template >
mla_bool_t mla_link_list_contains(const mla_link_list_t<T, TInit>& list, const T& item) {

    if (list.data == nullptr) {
        // If the list is empty, return false
        return false;
    }

    mla_link_list_node_t<T, TInit>* current = list.data->head;
    while (current) {
        if (current->data == item) {
            return true; // Item found in the list
        }
        current = current->next; // Move to the next node
    }
    return false; // Item not found in the list
}

template < mla_list_list_template >
mla_bool_t mla_link_list_get(const mla_link_list_t<T, TInit>& list, mla_size_t index, T& outValue) {

    if (list.data == nullptr) {
        outValue = TInit::init(); // If the list is empty, return the default value
        // If the list is empty, return false
        return false;
    }

    mla_link_list_node_t<T, TInit>* current = list.data->head;
    while (current) {

        if (index == 0) {
            outValue = current->data; // If the index is 0, return the current node's data
            return true;
        }

        index--; // Decrement the index
        current = current->next; // Move to the next node
    }

    outValue = TInit::init(); // If the index is out of bounds, return the default value
    return false;
}

template < mla_list_list_template >
T* mla_link_list_get_ref(const mla_link_list_t<T, TInit>& list, mla_size_t index) {

    if (list.data == nullptr) {
        // If the list is empty, return false
        return nullptr;
    }

    mla_link_list_node_t<T, TInit>* current = list.data->head;
    while (current) {

        if (index == 0) {
            return &current->data; // If the index is 0, return the current node's data
        }

        index--; // Decrement the index
        current = current->next; // Move to the next node
    }

    return nullptr;
}


template < mla_list_list_template >
mla_int32_t mla_link_list_index_of(const mla_link_list_t<T, TInit>& list, const T& item) {

    if (list.data == nullptr) {
        // If the list is empty, return false
        return -1;
    }

    mla_link_list_node_t<T, TInit> *current = list.data->head;
    mla_int32_t index = 0;
    while (current) {
        if (current->data == item) {
            return index; // Return the index of the item
        }
        current = current->next; // Move to the next node
        index++; // Increment the index
    }
    return -1; // Return -1 if the item is not found
}

template < mla_list_list_template >
inline void mla_link_list_clear(mla_link_list_t<T, TInit>& list) {

    if (list.data == nullptr) {
        // If the list is empty, initialize it
        list.data = __mla_link_list_data<T, TInit>();

        if (list.data == nullptr) {
            return; // Return if memory allocation fails
        }

        list.dataOwner = mla_buffer_reference(list.data, true, __mla_link_list_data_cleanup_hook<T, TInit>);
    }

    mla_link_list_destroy(list); // Clear the linked list by destroying it
    list = mla_link_list<T, TInit>(); // Reinitialize the list to an empty state
}

#endif

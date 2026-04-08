//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_LINK_LIST_H
#define COREOS_MLA_LINK_LIST_H

#include "../mla_data_types.h"
#include "mla_buffer.h"


#define mla_list_list_template typename  T, typename TInit = mla_default_init(T)


template < mla_list_list_template >
struct mla_link_list_node_t {
    mla_link_list_node_t<T, TInit>* next; // Pointer to the next node in the list
    mla_link_list_node_t<T, TInit>* prev; // Pointer to the previous node in the list
    mla_buffer_reference_t nextOwner;
    mla_buffer_reference_t prevOwner;
    T data;
};

template < mla_list_list_template >
struct mla_link_list_data_t {
    mla_link_list_node_t<T, TInit>* head; // Pointer to the first node in the list
    mla_link_list_node_t<T, TInit>* tail; // Pointer to the last node in the list
};

template < mla_list_list_template >
struct mla_link_list_t {

    mla_size_t size; // Number of elements in the list
    mla_buffer_reference_t headOwner; // Reference to the head node's buffer
    mla_buffer_reference_t tailOwner; // Reference to the tail node's buffer
    mla_link_list_data_t<T, TInit>* data;
    mla_buffer_reference_t dataOwner; // Reference to the data buffer
};

template < mla_list_list_template >
mla_buffer_cleanup_mode __mla_link_list_node_cleanup_hook(mla_pointer_t data, const mla_dynamic_data_t& userData) {

    (void)userData; // Silences the unused parameter warning

    // Custom cleanup hook for the linked list node
    mla_link_list_node_t<T, TInit>* node = static_cast<mla_link_list_node_t<T, TInit>*>(data);
    if (node) {
        node->data = TInit::init(); // Assign default value to trigger destructor if T is a class

    }

    return CLEAN_UP_NEEDED;
}

template < mla_list_list_template >
mla_buffer_cleanup_mode __mla_link_list_data_cleanup_hook(mla_pointer_t p_Data, const mla_dynamic_data_t& userData) {

    (void)userData; // Silences the unused parameter warning

    // Custom cleanup hook for the linked list node
    mla_link_list_data_t<T, TInit>* data = static_cast<mla_link_list_data_t<T, TInit>*>(p_Data);

    if (!data->head) {
        return CLEAN_UP_NEEDED;
    }

    // Iterate through the linked list and clean up each node
    mla_link_list_node_t<T, TInit>* current = data->head;
    while (current) {
        mla_link_list_node_t<T, TInit>* nextNode = current->next;
        current->nextOwner = mla_buffer_reference_noOwner();
        current->prevOwner = mla_buffer_reference_noOwner();
        current = nextNode; // Move to the next node
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
    return data; // Return the initialized data structure
}

template < mla_list_list_template >
inline mla_link_list_t<T, TInit>  mla_link_list_empty() {

    return {
        0,
        mla_buffer_reference_noOwner(),
        mla_buffer_reference_noOwner(),
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

    mla_link_list_t<T, TInit> list = { 0, mla_buffer_reference_noOwner(), mla_buffer_reference_noOwner(), data, mla_buffer_reference_create(data, false, __mla_link_list_data_cleanup_hook<T, TInit>, mla_dynamic_data_empty()) };
    return list; // Initialize an empty linked list
}

template < mla_list_list_template >
void mla_link_list_destroy(mla_link_list_t<T, TInit>& list) {

    list.size = 0; // Reset the size
    list.dataOwner = mla_buffer_reference_noOwner(); // Clear data owner reference
    list.headOwner = mla_buffer_reference_noOwner(); // Clear head owner reference
    list.tailOwner = mla_buffer_reference_noOwner(); // Clear tail owner reference
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

        list.dataOwner = mla_buffer_reference_create(list.data, false, __mla_link_list_data_cleanup_hook<T, TInit>, mla_dynamic_data_empty());
    }

    // Create a new node
    mla_link_list_node_t<T, TInit>* newNode = static_cast<mla_link_list_node_t<T, TInit>*>(mla_malloc(sizeof(mla_link_list_node_t<T, TInit>)));

    if (newNode == nullptr) {
        return false; // Return false if memory allocation fails
    }

    mla_memset(newNode, 0, sizeof(mla_link_list_node_t<T, TInit>)); // Initialize the new node
    mla_buffer_reference_t newNodeRef = mla_buffer_reference_create(newNode, false, __mla_link_list_node_cleanup_hook<T, TInit>, mla_dynamic_data_empty());

    newNode->data = item; // Set the data
    newNode->prev = list.data->tail; // Set previous pointer to the current tail
    newNode->prevOwner = list.tailOwner; // Initialize previous owner reference
    newNode->next = nullptr; // Set next pointer to null
    newNode->nextOwner = mla_buffer_reference_noOwner(); // Initialize next owner reference
    if (list.data->tail) {
        list.data->tail->next = newNode; // Link the new node to the current tail
        list.data->tail->nextOwner = newNodeRef;
    } else {
        list.data->head = newNode; // If the list was empty, set head to the new node
        list.headOwner = newNodeRef; // Initialize head owner reference
    }
    list.data->tail = newNode; // Update the tail to the new node
    list.tailOwner = newNodeRef;
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

        list.dataOwner = mla_buffer_reference_create(list.data, false, __mla_link_list_data_cleanup_hook<T, TInit>, mla_dynamic_data_empty());
    }

    mla_link_list_node_t<T, TInit>* current = list.data->head;
    while (current) {
        if (index == 0) {
            // If the node to remove is the head

            // Store the references to the next and previous owners before unlinking
            mla_buffer_reference_t current_nextOwner = current->nextOwner;
            mla_buffer_reference_t current_prevOwner = current->prevOwner;
            current->nextOwner = mla_buffer_reference_noOwner();
            current->prevOwner = mla_buffer_reference_noOwner();

            if (current->prev) {
                current->prev->next = current->next; // Link previous node to next node
                current->prev->nextOwner = current_nextOwner; // Update previous node's next owner reference
            } else {
                list.data->head = current->next; // Update head if removing the first node
                list.headOwner = current_nextOwner; // Update head owner reference
            }
            if (current->next) {
                current->next->prev = current->prev; // Link next node to previous node
                current->next->prevOwner = current_prevOwner; // Update next node's previous owner reference
            } else {
                list.data->tail = current->prev; // Update tail if removing the last node
                list.tailOwner = current_prevOwner; // Update tail owner reference
            }
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

        list.dataOwner = mla_buffer_reference_create(list.data, false, __mla_link_list_data_cleanup_hook<T, TInit>, mla_dynamic_data_empty());
    }

    mla_link_list_destroy(list); // Clear the linked list by destroying it
    list = mla_link_list<T, TInit>(); // Reinitialize the list to an empty state
}

#endif

//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_ARRAY_LIST_H
#define COREOS_MLA_ARRAY_LIST_H

#include "../mla_data_types.h"
#include "mla_buffer.h"

#define mla_array_list_default_capacity 16


#define mla_array_list_template typename  T, typename TInit = mla_default_init(T)

template <mla_array_list_template>
struct mla_array_list_t {
    mla_size_t size;
    mla_size_t capacity;
    T* items;
    mla_buffer_reference_t itemsOwner;
};

template <mla_array_list_template>
mla_buffer_cleanup_mode __mla_array_list_cleanup(mla_pointer_t p_Data, mla_callback_userdata p_UserData) {

    T* l_Item = static_cast<T*>(p_Data);

    for (mla_size_t i = 0; i < p_UserData; ++i) {
        l_Item[i] = TInit::init(); // Assign default value to trigger destructor if T is a class
    }

    return CLEAN_UP_NEEDED;
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list_empty() {

    return { 0, 0, nullptr, mla_buffer_reference_noOwner() };
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list(mla_size_t initialCapacity = mla_array_list_default_capacity) {

    mla_size_t size = initialCapacity * sizeof(T);
    T* items = static_cast<T*>(mla_malloc(size));
    mla_memset(items, 0, size);

    return { 0, initialCapacity, items, mla_buffer_reference(items, __mla_array_list_cleanup<T, TInit>, initialCapacity) };
}

template <mla_array_list_template>
void mla_array_list_destroy(mla_array_list_t<T, TInit>& list) {

    T* l_list = list.items;

    if (l_list != nullptr) {

        for (mla_size_t i = 0; i < list.size; ++i) {
            l_list[i] = TInit::init(); // Assign default value to trigger destructor if T is a class
        }

        list.items = nullptr;
    }


    list.size = 0;
    list.capacity = 0;
    list.itemsOwner = mla_buffer_reference_noOwner();
}

template <mla_array_list_template>
void mla_array_list_resize(mla_array_list_t<T, TInit>& list, mla_size_t newSize) {
    if (newSize >= list.size) {

        // Resize the array if the new size exceeds the current capacity
        T* newItems = static_cast<T*>(mla_malloc(newSize * sizeof(T)));

        if (list.items != nullptr) {
            mla_memcpy(newItems, list.items, list.size * sizeof(T));
        } else {
            mla_memset(newItems, 0, newSize * sizeof(T));
        }

        list.items = newItems;
        list.capacity = newSize;
        // We dont need to call the cleanup hook here because we are not destroying the old items
        // Instead, we just copy the existing items to the new array
        mla_buffer_reference_destroy_without_cleanup_unsafe(list.itemsOwner);
        list.itemsOwner = mla_buffer_reference(newItems, __mla_array_list_cleanup<T, TInit>, newSize); // Update the buffer reference
    }
}

template <mla_array_list_template>
mla_size_t mla_array_list_add(mla_array_list_t<T, TInit>& list, const T& item) {

    if (list.size >= list.capacity) {
        // Resize the array if necessary
        mla_size_t newCapacity = mla_max(list.capacity * 2, mla_array_list_default_capacity);
        mla_array_list_resize(list, newCapacity);
    }
    list.items[list.size++] = item; // Add the new item and increment the size
    return list.size - 1; // Return the index of the newly added item
}

template <mla_array_list_template>
mla_bool_t mla_array_list_get(const mla_array_list_t<T, TInit>& list, mla_size_t index, T& outItem) {

    if (index >= list.size) {
        outItem = TInit::init(); // Assign default value if index is out of bounds
        return false; // Return false if index is out of bounds
    }

    outItem = list.items[index]; // Return the item at the specified index
    return true;
}


template <mla_array_list_template>
T* mla_array_list_get_ref(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    if (index >= list.size) {
        return nullptr;
    }

    return &list.items[index]; // Return the item at the specified index
}

template <mla_array_list_template>
inline T& mla_array_list_get_unsafe(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    return list.items[index]; // Return the item at the specified index
}

template <mla_array_list_template>
inline mla_bool_t mla_array_list_remove(mla_array_list_t<T, TInit>& list, mla_size_t index) {
    if (index < list.size) {
        // Assign default value trigger destructor if T is a class
        list.items[index] = TInit::init();
        // Shift with memmove to remove the item at the specified index
        mla_memmove(list.items + index, list.items + index + 1, (list.size - index - 1) * sizeof(T));
        --list.size; // Decrement the size
        mla_memset(list.items + list.size, 0, sizeof(T)); // Clear the last item because it is no longer valid

        return true;
    }

    return false;
}

template <mla_array_list_template>
inline mla_size_t mla_array_list_size(const mla_array_list_t<T, TInit>& list) {
    return list.size; // Return the current size of the list
}

template <mla_array_list_template>
inline mla_size_t mla_array_list_capacity(const mla_array_list_t<T, TInit>& list) {
    return list.capacity; // Return the current capacity of the list
}

template <mla_array_list_template>
inline void mla_array_list_clear(mla_array_list_t<T, TInit>& list) {

    // assign default value to trigger destructor if T is a class
    T* l_list = list.items;
    for (mla_size_t i = 0; i < list.size; ++i) {
        l_list[i] = TInit::init();
    }

    list.size = 0;
}

template <mla_array_list_template>
inline void mla_array_list_shrink_to_fit(mla_array_list_t<T, TInit>& list) {
    mla_array_list_resize(list, list.size);
}

template <mla_array_list_template>
mla_bool_t mla_array_list_contains(const mla_array_list_t<T, TInit>& list, const T& item) {

    T* l_list = list.items;

    for (mla_size_t i = 0; i < list.size; ++i) {
        if (l_list[i] == item) {
            return true; // Item found
        }
    }
    return false; // Item not found
}

template <mla_array_list_template>
mla_int32_t mla_array_list_index_of(const mla_array_list_t<T, TInit>& list, const T& item) {
    T *l_list = list.items;
    for (mla_size_t i = 0; i < list.size; ++i) {
        if (l_list[i] == item) {
            return static_cast<mla_int32_t>(i); // Return the index of the item
        }
    }
    return -1;
}

template <mla_array_list_template>
struct mla_array_list_initializer {

    static mla_array_list_t<T, TInit> init() {
        return mla_array_list_empty<T, TInit>();
    }
};

#endif

//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_ARRAY_LIST_H
#define COREOS_MLA_ARRAY_LIST_H

#include "../mla_data_types.h"
#include "mla_buffer.h"

#define mla_array_list_template typename  T, typename TInit = mla_default_init(T)

template <mla_array_list_template>
struct mla_array_list_t {
    mla_size_t size;
    mla_size_t capacity;
    T* items;
    mla_buffer_reference_t itemsOwner;
};

template <mla_array_list_template>
mla_buffer_cleanup_mode __mla_array_list_cleanup(mla_pointer_t p_Data, const mla_dynamic_data_t& p_UserData) {

    T* l_Item = static_cast<T*>(p_Data);

    for (mla_size_t i = 0; i < p_UserData.asUint32; ++i) {
        l_Item[i] = TInit::init(); // Assign default value to trigger destructor if T is a class
    }

    return CLEAN_UP_NEEDED;
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list_empty() {

    return { 0, 0, nullptr, mla_buffer_reference_noOwner() };
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list(mla_size_t initialCapacity = mla_global_config_array_list_default_capacity) {

    if (initialCapacity == 0) {
        return mla_array_list_empty<T, TInit>();
    }

    mla_size_t size = initialCapacity * sizeof(T);
    T* items = static_cast<T*>(mla_malloc(size));

    if (items == nullptr) {
        return { 0, 0, nullptr, mla_buffer_reference_noOwner() }; // Memory allocation failed
    }

    mla_memset(items, 0, size);
    return { 0, initialCapacity, items, mla_buffer_reference_create(items, false, __mla_array_list_cleanup<T, TInit>, mla_dynamic_data_from_uint32(initialCapacity)) };
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
mla_bool_t mla_array_list_resize(mla_array_list_t<T, TInit>& list, mla_size_t newSize) {
    if (newSize >= list.size) {

        // Resize the array if the new size exceeds the current capacity
        mla_size_t newSizeInBytes = newSize * sizeof(T);
        T* newItems = static_cast<T*>(mla_malloc(newSizeInBytes));

        if (newItems == nullptr)
            return false; // Memory allocation failed

        if (list.items != nullptr) {
            mla_size_t oldSizeInBytes = list.size * sizeof(T);
            mla_memcpy(newItems, list.items, oldSizeInBytes);

            if (oldSizeInBytes < newSizeInBytes) {
                // Initialize the new elements to default value
                mla_memset(newItems + list.size, 0, newSizeInBytes - oldSizeInBytes);
            }

        } else {
            mla_memset(newItems, 0, newSizeInBytes);
        }

        mla_buffer_reference_t newItemsOwner = mla_buffer_reference_create(newItems, false, __mla_array_list_cleanup<T, TInit>, mla_dynamic_data_from_uint32(newSize));
        list.items = newItems;
        list.capacity = newSize;
        // We dont need to call the cleanup hook here because we are not destroying the old items
        // Instead, we just copy the existing items to the new array
        mla_buffer_reference_destroy_without_cleanup_unsafe(list.itemsOwner);
        list.itemsOwner = newItemsOwner; // Update the buffer reference
        return true;
    }

    return false;
}

template <mla_array_list_template>
mla_bool_t mla_array_list_add(mla_array_list_t<T, TInit>& list, const T& item) {

    if (list.size >= list.capacity) {
        // Resize the array if necessary
        mla_size_t newCapacity = mla_max(list.capacity * 2, mla_global_config_array_list_default_capacity);

        if (!mla_array_list_resize(list, newCapacity)) {
            return false; // Memory allocation failed
        }
    }
    list.items[list.size++] = item; // Add the new item and increment the size
    return true;
}

template <mla_array_list_template>
mla_bool_t mla_array_list_add_all(mla_array_list_t<T, TInit>& list, const mla_array_list_t<T, TInit>& newItems) {

    for (mla_size_t i = 0; i < newItems.size; ++i) {
        if (!mla_array_list_add(list, newItems.items[i])) {
            return false; // Memory allocation failed
        }
    }

    return true;
}

template <mla_array_list_template>
void mla_array_list_reverse(mla_array_list_t<T, TInit>& list) {

    if (list.size < 2) {
        return; // No need to reverse if the list has less than 2 items
    }

    mla_size_t start = 0;
    mla_size_t end = list.size - 1;

    while (start < end) {
        // Swap items at start and end
        T temp = list.items[start];
        list.items[start] = list.items[end];
        list.items[end] = temp;

        ++start;
        --end;
    }
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
T* mla_array_list_get_ref_unsafe(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    return &list.items[index]; // Return the item at the specified index
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
void __mla_array_list_quicksort_partition(T* items, mla_int32_t low, mla_int32_t high, mla_int32_t (*compare)(const T&, const T&)) {
    if (low < high) {
        // Choose pivot (middle element for better average performance)
        mla_int32_t mid = low + (high - low) / 2;
        T pivot = items[mid];

        // Move pivot to end
        T temp = items[mid];
        items[mid] = items[high];
        items[high] = temp;

        mla_int32_t i = low - 1;

        for (mla_int32_t j = low; j < high; ++j) {
            if (compare(items[j], pivot) <= 0) {
                ++i;
                // Swap items[i] and items[j]
                temp = items[i];
                items[i] = items[j];
                items[j] = temp;
            }
        }

        // Move pivot to correct position
        temp = items[i + 1];
        items[i + 1] = items[high];
        items[high] = temp;

        mla_int32_t partitionIndex = i + 1;

        // Recursively sort partitions
        __mla_array_list_quicksort_partition(items, low, partitionIndex - 1, compare);
        __mla_array_list_quicksort_partition(items, partitionIndex + 1, high, compare);
    }
}

template <mla_array_list_template>
void mla_array_list_sort(mla_array_list_t<T, TInit>& list, mla_int32_t (*compare)(const T&, const T&)) {
    if (list.size < 2) {
        return;
    }

    __mla_array_list_quicksort_partition<T, TInit>(list.items, 0, static_cast<mla_int32_t>(list.size - 1), compare);
}

template <mla_array_list_template>
struct mla_array_list_initializer {

    static mla_array_list_t<T, TInit> init() {
        return mla_array_list_empty<T, TInit>();
    }
};

#endif

//
// Created by chris on 8/2/2025.
//

#ifndef MLA_ARRAY_LIST_H
#define MLA_ARRAY_LIST_H

#include "../mla_data_types.h"

#define mla_array_list_template typename  T, typename TInit = mla_default_init(T)

template <mla_array_list_template>
struct mla_array_list_buffer_header_t {
    mla_size_t capacity;
    mla_byte_t _padding[4]; // newly added padding to force 8-byte size
};

template <mla_array_list_template>
struct mla_array_list_t {
    mla_size_t size;
    mla_pointer_t items;
};

template <mla_array_list_template>
inline T* mla_internal_array_list_items_data_from_header(mla_array_list_buffer_header_t<T, TInit>* header) {

    if (header == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<T*>(reinterpret_cast<mla_byte_t*>(header) + sizeof(mla_array_list_buffer_header_t<T, TInit>));
}

template <mla_array_list_template>
inline T* mla_internal_array_list_items_data_from_pointer(const mla_pointer_t& items_ptr) {

    mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(items_ptr);
    return mla_internal_array_list_items_data_from_header(header);
}

template <mla_array_list_template>
inline T* mla_internal_array_list_items_data(const mla_array_list_t<T, TInit>& list) {

    return mla_internal_array_list_items_data_from_pointer<T, TInit>(list.items);
}

template <mla_array_list_template>
void mla_internal_array_list_cleanup_header(mla_array_list_buffer_header_t<T, TInit>* header) {

    T* l_Item = mla_internal_array_list_items_data_from_header(header);

    for (mla_size_t i = 0; i < header->capacity; ++i) {
        l_Item[i] = TInit::init(); // Assign default value to trigger destructor if T is a class
    }

}

template <mla_array_list_template>
void mla_internal_array_list_cleanup(mla_platform_pointer_t p_Data, const mla_dynamic_data_t& p_UserData) {

    (void)p_UserData;

    mla_array_list_buffer_header_t<T, TInit>* header = static_cast<mla_array_list_buffer_header_t<T, TInit>*>(p_Data);
    mla_internal_array_list_cleanup_header<T, TInit>(header);
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list_empty() {

    return { 0, mla_pointer_null() };
}

template <mla_array_list_template>
mla_array_list_t<T, TInit> mla_array_list(mla_size_t initialCapacity = mla_global_config_array_list_default_capacity) {

    if (initialCapacity == 0) {
        return mla_array_list_empty<T, TInit>();
    }

    mla_size_t size = sizeof(mla_array_list_buffer_header_t<T, TInit>) + (initialCapacity * sizeof(T));
    mla_pointer_cleanup_hook_t cleanup_hook = mla_internal_array_list_cleanup<T, TInit>;
    mla_pointer_t items = mla_malloc_buffer_cleanup_hook(size, cleanup_hook);

    mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(items);

    if (header == nullptr) {
        return { 0, mla_pointer_null() }; // Memory allocation failed
    }

    header->capacity = initialCapacity;
    return { 0, items };
}

template <mla_array_list_template>
void mla_array_list_destroy(mla_array_list_t<T, TInit>& list) {

    mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(list.items);

    if (header != nullptr) {
        mla_internal_array_list_cleanup_header<T, TInit>(header);
        list.items = mla_pointer_null();
    }

    list.size = 0;
}

template <mla_array_list_template>
mla_bool_t mla_array_list_resize(mla_array_list_t<T, TInit>& list, mla_size_t newSize) {

    if (newSize >= list.size) {

        // Resize the array if the new size exceeds the current capacity
        mla_size_t newSizeInBytes = sizeof(mla_array_list_buffer_header_t<T, TInit>) + (newSize * sizeof(T));
        mla_pointer_cleanup_hook_t cleanup_hook = mla_internal_array_list_cleanup<T, TInit>;
        mla_pointer_t newItems_ptr = mla_malloc_buffer_cleanup_hook(newSizeInBytes, cleanup_hook);

        mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(newItems_ptr);

        if (header == nullptr) {
            return false; // Memory allocation failed
        }
        header->capacity = newSize;
        T* oldItems = mla_internal_array_list_items_data<T, TInit>(list);
        T* newItems = mla_internal_array_list_items_data_from_header<T, TInit>(header);

        if (newItems == nullptr) {
            return false; // Memory allocation failed
        }

        if (oldItems != nullptr) {
            mla_size_t oldSizeInBytes = list.size * sizeof(T);
            mla_memcpy(newItems, oldItems, oldSizeInBytes);
            mla_memset(oldItems, 0, oldSizeInBytes); // Clear old items via memset to dont tigger the constructors

        }

        list.items = newItems_ptr;
        return true;
    }

    return false;
}

template <mla_array_list_template>
inline mla_size_t mla_array_list_capacity(const mla_array_list_t<T, TInit>& list) {

    mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(list.items);

    if (header == nullptr) {
        return 0;
    }

    return header->capacity; // Return the current capacity of the list
}


template <mla_array_list_template>
mla_bool_t mla_array_list_add(mla_array_list_t<T, TInit>& list, const T& item) {

    mla_array_list_buffer_header_t<T, TInit>* header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(list.items);

    mla_size_t current_capacity;

    if (header != nullptr) {
        current_capacity = header->capacity;
    } else {
        current_capacity = 0;
    }

    if (list.size >= current_capacity) {
        // Resize the array if necessary
        mla_size_t newCapacity = mla_max(current_capacity * 2, mla_global_config_array_list_default_capacity);

        if (!mla_array_list_resize(list, newCapacity)) {
            return false; // Memory allocation failed
        }

        header = mla_pointer_get_data<mla_array_list_buffer_header_t<T, TInit>>(list.items);

    }

    T* items = mla_internal_array_list_items_data_from_header(header);

    items[list.size++] = item; // Add the new item and increment the size
    return true;
}

template <mla_array_list_template>
mla_bool_t mla_array_list_add_all(mla_array_list_t<T, TInit>& list, const mla_array_list_t<T, TInit>& newItems) {

    T* items_to_copy = mla_internal_array_list_items_data(newItems);

    for (mla_size_t i = 0; i < newItems.size; ++i) {

        if (!mla_array_list_add(list, items_to_copy[i])) {
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

    T* items = mla_internal_array_list_items_data(list);

    mla_size_t start = 0;
    mla_size_t end = list.size - 1;

    while (start < end) {
        // Swap items at start and end
        T temp = items[start];
        items[start] = items[end];
        items[end] = temp;

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

    T* items = mla_internal_array_list_items_data(list);

    outItem = items[index]; // Return the item at the specified index
    return true;
}

template <mla_array_list_template>
T* mla_array_list_get_ref_unsafe(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    T* items = mla_internal_array_list_items_data(list);
    return &items[index]; // Return the item at the specified index
}

template <mla_array_list_template>
T* mla_array_list_get_ref(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    if (index >= list.size) {
        return nullptr;
    }

    T* items = mla_internal_array_list_items_data(list);
    return &items[index]; // Return the item at the specified index
}

template <mla_array_list_template>
inline T& mla_array_list_get_unsafe(const mla_array_list_t<T, TInit>& list, mla_size_t index) {

    T* items = mla_internal_array_list_items_data(list);
    return items[index]; // Return the item at the specified index
}

template <mla_array_list_template>
inline mla_bool_t mla_array_list_remove(mla_array_list_t<T, TInit>& list, mla_size_t index) {

    if (index < list.size) {
        T* items = mla_internal_array_list_items_data(list);

        // Assign default value trigger destructor if T is a class
        items[index] = TInit::init();
        // Shift with memmove to remove the item at the specified index
        mla_memmove(items + index, items + index + 1, (list.size - index - 1) * sizeof(T));
        --list.size; // Decrement the size
        mla_memset(items + list.size, 0, sizeof(T)); // Clear the last item because it is no longer valid

        return true;
    }

    return false;
}

template <mla_array_list_template>
inline mla_size_t mla_array_list_size(const mla_array_list_t<T, TInit>& list) {
    return list.size; // Return the current size of the list
}

template <mla_array_list_template>
inline void mla_array_list_clear(mla_array_list_t<T, TInit>& list) {

    // assign default value to trigger destructor if T is a class
    T* items = mla_internal_array_list_items_data(list);
    for (mla_size_t i = 0; i < list.size; ++i) {
        items[i] = TInit::init();
    }

    list.size = 0;
}

template <mla_array_list_template>
inline void mla_array_list_shrink_to_fit(mla_array_list_t<T, TInit>& list) {
    mla_array_list_resize(list, list.size);
}

template <mla_array_list_template>
mla_bool_t mla_array_list_contains(const mla_array_list_t<T, TInit>& list, const T& item) {

    T* items = mla_internal_array_list_items_data(list);

    for (mla_size_t i = 0; i < list.size; ++i) {
        if (items[i] == item) {
            return true; // Item found
        }
    }
    return false; // Item not found
}

template <mla_array_list_template>
mla_int32_t mla_array_list_index_of(const mla_array_list_t<T, TInit>& list, const T& item) {

    T* items = mla_internal_array_list_items_data(list);

    for (mla_size_t i = 0; i < list.size; ++i) {
        if (items[i] == item) {
            return static_cast<mla_int32_t>(i); // Return the index of the item
        }
    }
    return -1;
}

template <mla_array_list_template>
void mla_internal_array_list_quicksort_partition(T* items, mla_int32_t low, mla_int32_t high, mla_int32_t (*compare)(const T&, const T&)) {
    if (low < high) {
        // Choose pivot (middle element for better average performance)
        mla_int32_t mid = low + ((high - low) / 2);
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
        mla_internal_array_list_quicksort_partition(items, low, partitionIndex - 1, compare);
        mla_internal_array_list_quicksort_partition(items, partitionIndex + 1, high, compare);
    }
}

template <mla_array_list_template>
void mla_array_list_sort(mla_array_list_t<T, TInit>& list, mla_int32_t (*compare)(const T&, const T&)) {
    if (list.size < 2) {
        return;
    }

    T* items = mla_internal_array_list_items_data(list);

    mla_internal_array_list_quicksort_partition<T, TInit>(items, 0, static_cast<mla_int32_t>(list.size - 1), compare);
}

template <mla_array_list_template>
struct mla_array_list_initializer {

    static mla_array_list_t<T, TInit> init() {
        return mla_array_list_empty<T, TInit>();
    }
};

#endif

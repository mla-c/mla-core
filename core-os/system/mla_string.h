//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_STRING_H
#define COREOS_MLA_STRING_H

#include "../mla_data_types.h"
#include "mla_buffer.h"
#include "../utils/mla_char_utils.h"



// Forward declaration
struct mla_multi_byte_char_t;
struct mla_string_t;

struct mla_c_string_t {
    const mla_char_t *c_str; // Pointer to the C-style string data
    const mla_bool_t isOwner; // Indicates if this string owns the data
};

// UTF-16 and UTF-32 string representations
// the data is owned by this struct.
// So you need to free it when you are done
struct mla_string_utf16_buffer_t {
    mla_utf_16_char_t *data; // Pointer to the UTF-16 string data
    mla_size_t charCount; // Length of the UTF-16 string without the null terminator
};

void mla_string_utf16_buffer_destroy(mla_string_utf16_buffer_t &p_Buffer);

struct mla_string_utf32_buffer_t {
    mla_utf_32_char_t *data; // Pointer to the UTF-32 string data
    mla_size_t charCount; // Length of the UTF-32 string without the null terminator
};

void mla_string_utf32_buffer_destroy(mla_string_utf32_buffer_t &p_Buffer);

enum mla_string_memory_layout_t {
    MLA_STRING_MEMORY_LAYOUT_C_STRING, // C-style string (with null terminator)
    MLA_STRING_MEMORY_LAYOUT_BUFFER, // Buffer-based string (without null terminator)
    MLA_STRING_MEMORY_LAYOUT_SUB_STRING // Substring (view into another string)
};

mla_char_t* mla_create_char_array(const mla_size_t p_Length);


struct mla_multi_byte_char_t {
    mla_char_t bytes[4]; // Maximum 4 bytes for a UTF-8 character. UTF-8 can be 1 to 4 bytes. Bytes will be null-terminated.
};


mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2);

struct mla_string_t {
    const mla_char_t *data; // Pointer to the string data
    mla_size_t length; // Buffer Length of the string. Not real Char count. This can be different in UTF8
    mla_buffer_reference_t dataOwner;
    mla_string_memory_layout_t memoryLayout;

    mla_bool_t operator==(const mla_string_t &other) const {

        mla_string_t thisString = *this;
        return mla_string_equals(thisString, other);
    }

    mla_bool_t operator!=(const mla_string_t &other) const {

        mla_string_t thisString = *this;
        return !mla_string_equals(thisString, other);
    }
};

mla_string_t mla_string_empty();
mla_string_t mla_string(const mla_char_t *p_Data, mla_size_t p_Length);
mla_string_t mla_string(const mla_char_t *p_Data);
mla_string_t mla_string(const mla_char_t *p_Data, const mla_char_t *p_End);


void mla_string_destroy(mla_string_t &p_String);

mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2);
mla_bool_t mla_string_equals_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2);

mla_bool_t mla_string_contains(const mla_string_t &p_String, const mla_string_t &p_Substring);

mla_bool_t mla_string_starts_with(const mla_string_t &p_String, const mla_string_t &p_Prefix);
mla_bool_t mla_string_ends_with(const mla_string_t &p_String, const mla_string_t &p_Suffix);

mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring);
mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring);

mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start, mla_size_t p_End);

// This function returns a multi-byte character at the specified index
// the most time its fine if you just access the buffer directly
// but if you want to be sure you get the right character in UTF8 use this function
mla_multi_byte_char_t mla_string_multi_byte_char_at(const mla_string_t &p_String, mla_size_t p_Index);
mla_size_t mla_string_multi_byte_char_count(const mla_string_t &p_String);

mla_string_utf16_buffer_t mla_string_to_utf16_buffer(mla_string_t &p_String);
mla_string_t mla_string_from_utf16_buffer(const mla_string_utf16_buffer_t &p_Utf16Buffer);

mla_string_utf32_buffer_t mla_string_to_utf32_buffer(mla_string_t &p_String);
mla_string_t mla_string_from_utf32_buffer(const mla_string_utf32_buffer_t &p_Utf32Buffer);

void mla_string_change_memory_layout(mla_string_t &p_String, mla_string_memory_layout_t p_NewLayout);

mla_c_string_t mla_string_to_cString(mla_string_t &p_String, mla_bool_t p_ForceCopy);

struct mla_string_initializer {

    static mla_string_t init() {
        return mla_string_empty();
    }
};


struct mla_string_hash_t {

    static mla_size_t hash(const mla_string_t& value) {

        // Simple hash function for strings
        mla_size_t hash = 5381;
        for (mla_size_t i = 0; i < value.length; ++i) {
            hash = ((hash << 5) + hash) + value.data[i]; // hash = hash * 33 + value.data[i];
        }
        return hash;


    }

};

#define mla_string_const(VALUE) mla_string(VALUE, sizeof(VALUE) -1 )


#endif //COREOS_MLA_STRING_H

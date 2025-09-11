//
// Created by christian on 9/10/25.
//


#include "mla_string_concat.h"


mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2) {

    mla_size_t size1 = mla_strlen(p_String2);
    mla_size_t newLength = p_String1.length + size1;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2, size1);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t newLength = p_String1.length + p_String2.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2.data, p_String2.length);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, mla_int32_t number) {

    mla_format(buffer, 12, "%d", number);
    mla_size_t size1 = mla_strlen(p_String1);

    mla_size_t newLength = size1 + 12;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1, size1);
    mla_memcpy(newData + size1, buffer, 12);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t newLength = size1 + p_String2.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1, size1);
    mla_memcpy(newData + size1, p_String2.data, p_String2.length);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3) {

    mla_size_t newLength = p_String1.length + p_String2.length + p_String3.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2.data, p_String2.length);
    mla_memcpy(newData + p_String1.length + p_String2.length, p_String3.data, p_String3.length);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t newLength = size1 + p_String2.length + size3;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1, size1);
    mla_memcpy(newData + size1, p_String2.data, p_String2.length);
    mla_memcpy(newData + size1 + p_String2.length, p_String3, size3);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3,
                  const mla_string_t &p_String4) {

    mla_size_t newLength = p_String1.length + p_String2.length + p_String3.length + p_String4.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2.data, p_String2.length);
    mla_memcpy(newData + p_String1.length + p_String2.length, p_String3.data, p_String3.length);
    mla_memcpy(newData + p_String1.length + p_String2.length + p_String3.length, p_String4.data, p_String4.length);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4 ) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t newLength = size1 + p_String2.length + size3 + size4;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1, size1);
    mla_memcpy(newData + size1, p_String2.data, p_String2.length);
    mla_memcpy(newData + size1 + p_String2.length, p_String3, size3);
    mla_memcpy(newData + size1 + p_String2.length + size3, p_String4, size4);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}
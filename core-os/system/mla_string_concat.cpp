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

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2, const mla_string_t &p_String3) {

    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t newLength = p_String1.length + size2 + p_String3.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2, size2);
    mla_memcpy(newData + p_String1.length + size2, p_String3.data, p_String3.length);
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

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4 ) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t newLength = size1 + p_String2.length + size3 + p_String4.length;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1, size1);
    mla_memcpy(newData + size1, p_String2.data, p_String2.length);
    mla_memcpy(newData + size1 + p_String2.length, p_String3, size3);
    mla_memcpy(newData + size1 + p_String2.length + size3, p_String4.data, p_String4.length);
    newData[newLength] = '\0'; // Null-terminate the string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};
}

mla_string_t mla_string_concat(const mla_string_t& p_String1, const mla_char_t* p_String2, const mla_string_t&  p_String3, const mla_char_t* p_String4 ) {

    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t newLength = p_String1.length + size2 + p_String3.length + size4;
    mla_char_t *newData = mla_create_char_array(newLength + 1);
    mla_memcpy(newData, p_String1.data, p_String1.length);
    mla_memcpy(newData + p_String1.length, p_String2, size2);
    mla_memcpy(newData + p_String1.length + size2, p_String3.data, p_String3.length);
    mla_memcpy(newData + p_String1.length + size2 + p_String3.length, p_String4, size4);
    newData[newLength] = '\0'; // Null-terminate the string if it's a C-style string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};

}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5 ) {

    // Calculate lengths of C-style strings
    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t size5 = mla_strlen(p_String5);

    // Calculate total length
    mla_size_t newLength = size1 + p_String2.length + size3 + p_String4.length + size5;

    // Allocate memory
    mla_char_t *newData = mla_create_char_array(newLength + 1);

    // Copy strings
    mla_size_t offset = 0;
    mla_memcpy(newData + offset, p_String1, size1);
    offset += size1;

    mla_memcpy(newData + offset, p_String2.data, p_String2.length);
    offset += p_String2.length;

    mla_memcpy(newData + offset, p_String3, size3);
    offset += size3;

    mla_memcpy(newData + offset, p_String4.data, p_String4.length);
    offset += p_String4.length;

    mla_memcpy(newData + offset, p_String5, size5);

    // Null-terminate the string
    newData[newLength] = '\0';

    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};


}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t& p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4, const mla_char_t* p_String5, const mla_char_t* p_String6, const mla_char_t* p_String7, const mla_char_t* p_String8, const mla_char_t* p_String9) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t size6 = mla_strlen(p_String6);
    mla_size_t size7 = mla_strlen(p_String7);
    mla_size_t size8 = mla_strlen(p_String8);
    mla_size_t size9 = mla_strlen(p_String9);

    mla_size_t newLength = size1 + p_String2.length + size3 + size4 + size5 + size6 + size7 + size8 + size9;
    mla_char_t *newData = mla_create_char_array(newLength + 1);

    mla_size_t offset = 0;
    mla_memcpy(newData + offset, p_String1, size1);
    offset += size1;

    mla_memcpy(newData + offset, p_String2.data, p_String2.length);
    offset += p_String2.length;

    mla_memcpy(newData + offset, p_String3, size3);
    offset += size3;

    mla_memcpy(newData + offset, p_String4, size4);
    offset += size4;

    mla_memcpy(newData + offset, p_String5, size5);
    offset += size5;

    mla_memcpy(newData + offset, p_String6, size6);
    offset += size6;

    mla_memcpy(newData + offset, p_String7, size7);
    offset += size7;

    mla_memcpy(newData + offset, p_String8, size8);
    offset += size8;

    mla_memcpy(newData + offset, p_String9, size9);

    newData[newLength] = '\0'; // Null-terminate the string
    return { newData, newLength, mla_buffer_reference(newData), MLA_STRING_MEMORY_LAYOUT_C_STRING};

}
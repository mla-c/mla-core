//
// Created by christian on 9/10/25.
//


#include "mla_string_concat.h"


mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t newLength = length1 + size2;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, p_String2, size2);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, p_String2, size2);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t newLength = length1 + length2;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data2 = mla_string_data(p_String2);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, data2, length2);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, data2, length2);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t newLength = size1 + length2;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        mla_memcpy(result.embedded.data, p_String1, size1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        mla_memcpy(result.embedded.data + size1, data2, length2);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    mla_memcpy(new_data_ptr, p_String1, size1);
    const mla_char_t* data2 = mla_string_data(p_String2);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_char_t* p_String2) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t newLength = size1 + size2;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, p_String2, size2);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, p_String2, size2);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t newLength = length1 + length2 + length3;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, data2, length2);
        mla_memcpy(result.embedded.data + length1 + length2, data3, length3);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, data2, length2);
    mla_memcpy(new_data_ptr + length1 + length2, data3, length3);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t newLength = size1 + length2 + size3;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        mla_memcpy(result.embedded.data, p_String1, size1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t newLength = size1 + length2 + length3;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, data3, length3);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, data3, length3);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;

}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_char_t* p_String2, const mla_string_t &p_String3) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t newLength = length1 + size2 + length3;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data3 = mla_string_data(p_String3);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, p_String2, size2);
        mla_memcpy(result.embedded.data + length1 + size2, data3, length3);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data3 = mla_string_data(p_String3);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, p_String2, size2);
    mla_memcpy(new_data_ptr + length1 + size2, data3, length3);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_char_t* p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t newLength = size1 + size2 + size3 + length4;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data4 = mla_string_data(p_String4);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, p_String2, size2);
        mla_memcpy(result.embedded.data + size1 + size2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + size2 + size3, data4, length4);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data4 = mla_string_data(p_String4);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, p_String2, size2);
    mla_memcpy(new_data_ptr + size1 + size2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + size2 + size3, data4, length4);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3, const mla_string_t &p_String4) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t newLength = length1 + length2 + length3 + length4;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);
        const mla_char_t* data4 = mla_string_data(p_String4);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, data2, length2);
        mla_memcpy(result.embedded.data + length1 + length2, data3, length3);
        mla_memcpy(result.embedded.data + length1 + length2 + length3, data4, length4);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);
    const mla_char_t* data4 = mla_string_data(p_String4);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, data2, length2);
    mla_memcpy(new_data_ptr + length1 + length2, data3, length3);
    mla_memcpy(new_data_ptr + length1 + length2 + length3, data4, length4);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t newLength = size1 + length2 + size3 + size4;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, p_String4, size4);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, p_String4, size4);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4 ) {

    mla_size_t size1   = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3   = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t newLength = size1 + length2 + size3 + length4;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;

}

mla_string_t mla_string_concat(const mla_string_t& p_String1, const mla_char_t* p_String2, const mla_string_t&  p_String3, const mla_char_t* p_String4 ) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t size2   = mla_strlen(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t size4   = mla_strlen(p_String4);
    mla_size_t newLength = length1 + size2 + length3 + size4;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data3 = mla_string_data(p_String3);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, p_String2, size2);
        mla_memcpy(result.embedded.data + length1 + size2, data3, length3);
        mla_memcpy(result.embedded.data + length1 + size2 + length3, p_String4, size4);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data3 = mla_string_data(p_String3);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, p_String2, size2);
    mla_memcpy(new_data_ptr + length1 + size2, data3, length3);
    mla_memcpy(new_data_ptr + length1 + size2 + length3, p_String4, size4);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;

}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_string_t& p_String3, const mla_char_t* p_String4, const mla_string_t& p_String5 ) {

    mla_size_t size1   = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t size4   = mla_strlen(p_String4);
    mla_size_t length5 = mla_string_length(p_String5);
    mla_size_t newLength = size1 + length2 + length3 + size4 + length5;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);
        const mla_char_t* data5 = mla_string_data(p_String5);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, data3, length3);
        mla_memcpy(result.embedded.data + size1 + length2 + length3, p_String4, size4);
        mla_memcpy(result.embedded.data + size1 + length2 + length3 + size4, data5, length5);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);
    const mla_char_t* data5 = mla_string_data(p_String5);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, data3, length3);
    mla_memcpy(new_data_ptr + size1 + length2 + length3, p_String4, size4);
    mla_memcpy(new_data_ptr + size1 + length2 + length3 + size4, data5, length5);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3, const mla_string_t &p_String4, const mla_string_t &p_String5 ) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t length5 = mla_string_length(p_String5);
    mla_size_t newLength = length1 + length2 + length3 + length4 + length5;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data5 = mla_string_data(p_String5);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, data2, length2);
        mla_memcpy(result.embedded.data + length1 + length2, data3, length3);
        mla_memcpy(result.embedded.data + length1 + length2 + length3, data4, length4);
        mla_memcpy(result.embedded.data + length1 + length2 + length3 + length4, data5, length5);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data5 = mla_string_data(p_String5);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, data2, length2);
    mla_memcpy(new_data_ptr + length1 + length2, data3, length3);
    mla_memcpy(new_data_ptr + length1 + length2 + length3, data4, length4);
    mla_memcpy(new_data_ptr + length1 + length2 + length3 + length4, data5, length5);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;

}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t newLength = size1 + length2 + size3 + length4 + size5;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4, p_String5, size5);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4, p_String5, size5);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_char_t* p_String2, const mla_string_t& p_String3, const mla_char_t* p_String4, const mla_string_t& p_String5, const mla_char_t* p_String6 ) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t size2 = mla_strlen(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t length5 = mla_string_length(p_String5);
    mla_size_t size6 = mla_strlen(p_String6);
    mla_size_t newLength = size1 + size2 + length3 + size4 + length5 + size6;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data3 = mla_string_data(p_String3);
        const mla_char_t* data5 = mla_string_data(p_String5);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, p_String2, size2);
        mla_memcpy(result.embedded.data + size1 + size2, data3, length3);
        mla_memcpy(result.embedded.data + size1 + size2 + length3, p_String4, size4);
        mla_memcpy(result.embedded.data + size1 + size2 + length3 + size4, data5, length5);
        mla_memcpy(result.embedded.data + size1 + size2 + length3 + size4 + length5, p_String6, size6);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data3 = mla_string_data(p_String3);
    const mla_char_t* data5 = mla_string_data(p_String5);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, p_String2, size2);
    mla_memcpy(new_data_ptr + size1 + size2, data3, length3);
    mla_memcpy(new_data_ptr + size1 + size2 + length3, p_String4, size4);
    mla_memcpy(new_data_ptr + size1 + size2 + length3 + size4, data5, length5);
    mla_memcpy(new_data_ptr + size1 + size2 + length3 + size4 + length5, p_String6, size6);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_string_t &p_String1, const mla_string_t &p_String2, const mla_string_t &p_String3, const mla_string_t &p_String4, const mla_string_t &p_String5, const mla_string_t &p_String6 ) {

    mla_size_t length1 = mla_string_length(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t length3 = mla_string_length(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t length5 = mla_string_length(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t newLength = length1 + length2 + length3 + length4 + length5 + length6;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data1 = mla_string_data(p_String1);
        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data3 = mla_string_data(p_String3);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data5 = mla_string_data(p_String5);
        const mla_char_t* data6 = mla_string_data(p_String6);

        mla_memcpy(result.embedded.data, data1, length1);
        mla_memcpy(result.embedded.data + length1, data2, length2);
        mla_memcpy(result.embedded.data + length1 + length2, data3, length3);
        mla_memcpy(result.embedded.data + length1 + length2 + length3, data4, length4);
        mla_memcpy(result.embedded.data + length1 + length2 + length3 + length4, data5, length5);
        mla_memcpy(result.embedded.data + length1 + length2 + length3 + length4 + length5, data6, length6);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data1 = mla_string_data(p_String1);
    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data3 = mla_string_data(p_String3);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data5 = mla_string_data(p_String5);
    const mla_char_t* data6 = mla_string_data(p_String6);

    mla_memcpy(new_data_ptr, data1, length1);
    mla_memcpy(new_data_ptr + length1, data2, length2);
    mla_memcpy(new_data_ptr + length1 + length2, data3, length3);
    mla_memcpy(new_data_ptr + length1 + length2 + length3, data4, length4);
    mla_memcpy(new_data_ptr + length1 + length2 + length3 + length4, data5, length5);
    mla_memcpy(new_data_ptr + length1 + length2 + length3 + length4 + length5, data6, length6);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6 ) {

    mla_size_t size1   = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3   = mla_strlen(p_String3);
    mla_size_t size4   = mla_strlen(p_String4);
    mla_size_t size5   = mla_strlen(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t newLength = size1 + length2 + size3 + size4 + size5 + length6;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data6 = mla_string_data(p_String6);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, p_String4, size4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4 + size5, data6, length6);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data6 = mla_string_data(p_String6);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, p_String4, size4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4 + size5, data6, length6);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}


mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t newLength = size1 + length2 + size3 + length4 + size5 + length6;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data6 = mla_string_data(p_String6);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5, data6, length6);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data6 = mla_string_data(p_String6);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5, data6, length6);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t size7 = mla_strlen(p_String7);
    mla_size_t newLength = size1 + length2 + size3 + length4 + size5 + length6 + size7;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data6 = mla_string_data(p_String6);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5, data6, length6);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data6 = mla_string_data(p_String6);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5, data6, length6);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t &p_String2, const mla_char_t* p_String3, const mla_string_t &p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7, const mla_string_t& p_String8) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t size7 = mla_strlen(p_String7);
    mla_size_t length8 = mla_string_length(p_String8);
    mla_size_t newLength = size1 + length2 + size3 + length4 + size5 + length6 + size7 + length8;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data6 = mla_string_data(p_String6);
        const mla_char_t* data8 = mla_string_data(p_String8);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5, data6, length6);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6 + size7, data8, length8);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);
    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data6 = mla_string_data(p_String6);
    const mla_char_t* data8 = mla_string_data(p_String8);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5, data6, length6);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6 + size7, data8, length8);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t& p_String2, const mla_char_t* p_String3, const mla_char_t* p_String4, const mla_char_t* p_String5, const mla_char_t* p_String6, const mla_char_t* p_String7, const mla_char_t* p_String8, const mla_char_t* p_String9) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t size4 = mla_strlen(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t size6 = mla_strlen(p_String6);
    mla_size_t size7 = mla_strlen(p_String7);
    mla_size_t size8 = mla_strlen(p_String8);
    mla_size_t size9 = mla_strlen(p_String9);
    mla_size_t newLength = size1 + length2 + size3 + size4 + size5 + size6 + size7 + size8 + size9;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, p_String4, size4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4 + size5, p_String6, size6);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4 + size5 + size6, p_String7, size7);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4 + size5 + size6 + size7, p_String8, size8);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + size4 + size5 + size6 + size7 + size8, p_String9, size9);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);
    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, p_String4, size4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4 + size5, p_String6, size6);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4 + size5 + size6, p_String7, size7);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4 + size5 + size6 + size7, p_String8, size8);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + size4 + size5 + size6 + size7 + size8, p_String9, size9);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}

mla_string_t mla_string_concat(const mla_char_t* p_String1, const mla_string_t& p_String2, const mla_char_t* p_String3, const mla_string_t& p_String4, const mla_char_t* p_String5, const mla_string_t& p_String6, const mla_char_t* p_String7,  const mla_string_t& p_String8, const mla_char_t* p_String9) {

    mla_size_t size1 = mla_strlen(p_String1);
    mla_size_t length2 = mla_string_length(p_String2);
    mla_size_t size3 = mla_strlen(p_String3);
    mla_size_t length4 = mla_string_length(p_String4);
    mla_size_t size5 = mla_strlen(p_String5);
    mla_size_t length6 = mla_string_length(p_String6);
    mla_size_t size7 = mla_strlen(p_String7);
    mla_size_t length8 = mla_string_length(p_String8);
    mla_size_t size9 = mla_strlen(p_String9);
    mla_size_t newLength = size1 + length2 + size3 + length4 + size5 + length6 + size7 + length8 + size9;

    // Use small string optimization if the result fits
    if (newLength <= mla_global_config_string_sso_max_length) {
        mla_string_t result = {mla_pointer_null(), {{MLA_STRING_MEMORY_LAYOUT_EMBEDDED, 0, {0}}}};
        result.embedded.length = mla_s_cast<mla_uint8_t>(newLength);

        const mla_char_t* data2 = mla_string_data(p_String2);
        const mla_char_t* data4 = mla_string_data(p_String4);
        const mla_char_t* data6 = mla_string_data(p_String6);
        const mla_char_t* data8 = mla_string_data(p_String8);

        mla_memcpy(result.embedded.data, p_String1, size1);
        mla_memcpy(result.embedded.data + size1, data2, length2);
        mla_memcpy(result.embedded.data + size1 + length2, p_String3, size3);
        mla_memcpy(result.embedded.data + size1 + length2 + size3, data4, length4);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4, p_String5, size5);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5, data6, length6);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6 + size7, data8, length8);
        mla_memcpy(result.embedded.data + size1 + length2 + size3 + length4 + size5 + length6 + size7 + length8, p_String9, size9);

        return result;
    }

    // Fall back to heap allocation for larger strings
    mla_pointer_t newData = mla_create_char_array(newLength + 1);

    mla_char_t* new_data_ptr = mla_pointer_get_data<mla_char_t>(newData);

    if (new_data_ptr == nullptr) {
        return mla_string_const("Concat Failed - Out of Memory");
    }

    const mla_char_t* data2 = mla_string_data(p_String2);
    const mla_char_t* data4 = mla_string_data(p_String4);
    const mla_char_t* data6 = mla_string_data(p_String6);
    const mla_char_t* data8 = mla_string_data(p_String8);

    mla_memcpy(new_data_ptr, p_String1, size1);
    mla_memcpy(new_data_ptr + size1, data2, length2);
    mla_memcpy(new_data_ptr + size1 + length2, p_String3, size3);
    mla_memcpy(new_data_ptr + size1 + length2 + size3, data4, length4);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4, p_String5, size5);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5, data6, length6);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6, p_String7, size7);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6 + size7, data8, length8);
    mla_memcpy(new_data_ptr + size1 + length2 + size3 + length4 + size5 + length6 + size7 + length8, p_String9, size9);
    new_data_ptr[newLength] = '\0';

    mla_string_t result = {newData, {{MLA_STRING_MEMORY_LAYOUT_C_STRING, 0, {0}}}};
    result.heap.char_offset = 0;
    result.heap.length = newLength;
    return result;
}


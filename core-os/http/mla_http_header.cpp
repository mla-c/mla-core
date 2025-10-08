//
// Created by christian on 10/6/25.
//

#include "mla_http_header.h"

mla_http_header_t mla_http_header_empty() {
    return {
        mla_string_empty(),
        MLA_HTTP_HEADER_TYPE_SINGLE,
        mla_array_list_empty<mla_string_t, mla_string_initializer>(),
        mla_string_empty()
    };
}

void mla_http_headers_add(mla_array_list_t<mla_http_header_t, mla_http_header_initializer> &p_Headers, const mla_string_t &p_Name, const mla_string_t &p_Value) {

    // Check if header already exists
    for (mla_size_t i = 0; i < p_Headers.size; i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, add value
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                // Convert to multi-value header
                mla_string_t existing_value = header->value;
                header->value = mla_string_empty();
                header->type = MLA_HTTP_HEADER_TYPE_MULTI;
                header->values = mla_array_list<mla_string_t, mla_string_initializer>();
                mla_array_list_add(header->values, existing_value);
            }

            // Add new value
            mla_array_list_add(header->values, p_Value);
            return;
        }
    }

    // Header does not exist, create new
    mla_http_header_t new_header = {
        p_Name,
        MLA_HTTP_HEADER_TYPE_SINGLE,
        mla_array_list_empty<mla_string_t, mla_string_initializer>(),
        p_Value,

    };

    mla_array_list_add(p_Headers, new_header);
}

mla_string_t mla_http_headers_get_value(const mla_array_list_t<mla_http_header_t, mla_http_header_initializer> &p_Headers, const mla_string_t &p_Name) {

    // Check if header already exists
    for (mla_size_t i = 0; i < p_Headers.size; i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, add value
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                return header->value;
            }

            // Return first value for multi-value header
            if (mla_array_list_size(header->values) > 0) {
                return mla_array_list_get_unsafe(header->values, 0);
            }

        }
    }

    return mla_string_empty();

}

mla_array_list_t<mla_string_t, mla_string_initializer> mla_http_headers_get_values(const mla_array_list_t<mla_http_header_t, mla_http_header_initializer> &p_Headers, const mla_string_t &p_Name) {

    // Check if header already exists
    for (mla_size_t i = 0; i < p_Headers.size; i++) {

        mla_http_header_t* header = mla_array_list_get_ref(p_Headers, i);

        if (mla_string_equals_ignore_case(header->name, p_Name)) {

            // Header exists, return values
            if (header->type == MLA_HTTP_HEADER_TYPE_SINGLE) {
                mla_array_list_t<mla_string_t, mla_string_initializer> values = mla_array_list<mla_string_t, mla_string_initializer>(1);
                mla_array_list_add(values, header->value);
                return values;
            }

            // Return all values for multi-value header
            return header->values;
        }
    }

    return mla_array_list_empty<mla_string_t, mla_string_initializer>();
}
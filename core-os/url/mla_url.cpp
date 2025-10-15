//
// Created by christian on 10/8/25.
//

#include "mla_url.h"

#include "../system/mla_number.h"

mla_url_query_param_t mla_url_query_param_empty() {
    return {
        mla_string_empty(),
        mla_string_empty()
    };
}

mla_url_t mla_url_empty() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        0,
        mla_string_empty(),
        mla_array_list_empty<mla_url_query_param_t, mla_url_query_param_initializer>(),
        mla_string_empty()
    };
}

mla_bool_t mla_url_parse(const mla_string_t &urlString, mla_url_t &outUrl) {

    if (mla_string_is_empty(urlString)) {
        return false;
    }

    mla_size_t pos = 0;

    // Parse scheme (protocol)
    mla_string_t schemeDelimiter = mla_string_const("://");
    mla_int32_t schemeEndPos = mla_string_index_of(urlString, schemeDelimiter);
    if (schemeEndPos == -1) {
        return false; // Invalid URL without scheme
    }

    outUrl.scheme = mla_string_substr(urlString, 0, schemeEndPos);
    pos = schemeEndPos + 3; // Skip "://"

    // Find the end of host (could be '/', '?', '#', or end of string)
    mla_size_t hostStart = pos;
    mla_size_t pathStart = pos;

    while (pathStart < urlString.length &&
           urlString.data[pathStart] != '/' &&
           urlString.data[pathStart] != '?' &&
           urlString.data[pathStart] != '#') {
        pathStart++;
    }

    // Check for port
    mla_int32_t portPos = -1;
    for (mla_size_t p = hostStart; p < pathStart; p++) {
        if (urlString.data[p] == ':') {
            portPos = p;
            break;
        }
    }

    if (portPos != -1) {
        outUrl.host = mla_string_substr(urlString, hostStart, portPos - hostStart);
        // Parse port number
        mla_string_t portString = mla_string_substr(urlString, portPos + 1, pathStart - portPos - 1);
        mla_uint16_t portValue = 0;
        if (!mla_parse_uint16(portString, portValue)) {
            mla_string_destroy(portString);
            return false; // Invalid port
        }
        mla_string_destroy(portString);
        outUrl.port = portValue;
    } else {
        outUrl.host = mla_string_substr(urlString, hostStart, pathStart - hostStart);

        // default ports based on scheme
        if (mla_string_equals_const(outUrl.scheme, "http")) {
            outUrl.port = 80;
        } else if (mla_string_equals_const(outUrl.scheme, "https")) {
            outUrl.port = 443;
        } else if (mla_string_equals_const(outUrl.scheme, "ws")) {
            outUrl.port = 80;
        } else if (mla_string_equals_const(outUrl.scheme, "wss")) {
            outUrl.port = 443;
        } else {
            outUrl.port = 0; // Unknown scheme, port not set
        }
    }

    pos = pathStart;

    // Parse path
    if (pos < urlString.length && urlString.data[pos] == '/') {
        mla_size_t pathEnd = pos;
        while (pathEnd < urlString.length &&
               urlString.data[pathEnd] != '?' &&
               urlString.data[pathEnd] != '#') {
            pathEnd++;
        }
        outUrl.path = mla_string_substr(urlString, pos, pathEnd - pos);
        pos = pathEnd;
    } else {
        outUrl.path = mla_string_empty();
    }

    // Parse query parameters
    if (pos < urlString.length && urlString.data[pos] == '?') {
        pos++; // Skip '?'
        mla_size_t queryEnd = pos;
        while (queryEnd < urlString.length && urlString.data[queryEnd] != '#') {
            queryEnd++;
        }

        // Parse individual query parameters
        mla_size_t paramStart = pos;
        while (paramStart < queryEnd) {
            mla_size_t equalSign = paramStart;
            while (equalSign < queryEnd &&
                   urlString.data[equalSign] != '=' &&
                   urlString.data[equalSign] != '&') {
                equalSign++;
            }

            mla_url_query_param_t param = mla_url_query_param_empty();
            param.key = mla_string_substr(urlString, paramStart, equalSign - paramStart);

            if (equalSign < queryEnd && urlString.data[equalSign] == '=') {
                mla_size_t valueStart = equalSign + 1;
                mla_size_t valueEnd = valueStart;
                while (valueEnd < queryEnd && urlString.data[valueEnd] != '&') {
                    valueEnd++;
                }
                param.value = mla_string_substr(urlString, valueStart, valueEnd - valueStart);
                paramStart = valueEnd;
            } else {
                paramStart = equalSign;
            }

            if (paramStart < queryEnd && urlString.data[paramStart] == '&') {
                paramStart++;
            }

            mla_array_list_add(outUrl.query, param);
        }

        pos = queryEnd;
    }

    // Parse fragment
    if (pos < urlString.length && urlString.data[pos] == '#') {
        pos++; // Skip '#'
        outUrl.fragment = mla_string_substr(urlString, pos);
    } else {
        outUrl.fragment = mla_string_empty();
    }

    return true;
}

mla_string_t mla_url_to_string(const mla_url_t &url) {

    // Calculate required buffer size
    mla_size_t totalLength = url.scheme.length + 3; // "://"
    totalLength += url.host.length;

    if (url.port > 0) {
        totalLength += 6; // ":" + max 5 digits
    }

    totalLength += mla_max(url.path.length, 1); // Ensure at least 1 for "/"

    if (mla_array_list_size(url.query) > 0) {
        totalLength += 1; // "?"
        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            totalLength += param->key.length + 1 + param->value.length; // key=value

            if (i < mla_array_list_size(url.query) - 1) {
                totalLength += 1; // "&"
            }
        }
    }

    if (!mla_string_is_empty(url.fragment)) {
        totalLength += 1 + url.fragment.length; // "#" + fragment
    }

    // Allocate buffer
    mla_char_t *buffer = mla_create_char_array(totalLength);

    if (buffer == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_size_t offset = 0;

    // Build URL string
    mla_memcpy(buffer + offset, url.scheme.data, url.scheme.length);
    offset += url.scheme.length;

    mla_memcpy(buffer + offset, "://", 3);
    offset += 3;

    mla_memcpy(buffer + offset, url.host.data, url.host.length);
    offset += url.host.length;

    if (url.port > 0) {
        mla_string_t portStr = mla_string_from_uint16(url.port);
        buffer[offset++] = ':';
        mla_memcpy(buffer + offset, portStr.data, portStr.length);
        offset += portStr.length;
        mla_string_destroy(portStr);
    }

    if (url.path.length == 0) {
        buffer[offset++] = '/';
    } else {
        mla_memcpy(buffer + offset, url.path.data, url.path.length);
        offset += url.path.length;
    }

    if (mla_array_list_size(url.query) > 0) {
        buffer[offset++] = '?';

        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            mla_memcpy(buffer + offset, param->key.data, param->key.length);
            offset += param->key.length;
            buffer[offset++] = '=';
            mla_memcpy(buffer + offset, param->value.data, param->value.length);
            offset += param->value.length;
            if (i < mla_array_list_size(url.query) - 1) {
                buffer[offset++] = '&';
            }
        }
    }

    if (!mla_string_is_empty(url.fragment)) {
        buffer[offset++] = '#';
        mla_memcpy(buffer + offset, url.fragment.data, url.fragment.length);
        offset += url.fragment.length;
    }

    return mla_string_from_buffer_with_ownership(buffer, offset);
}

mla_string_t mla_url_to_string_pathAndQuery(const mla_url_t &url) {

    mla_size_t totalLength = mla_max(url.path.length, 1); // Default to "/" if empty

    if (mla_array_list_size(url.query) > 0) {
        totalLength += 1; // "?"
        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            totalLength += param->key.length + 1 + param->value.length; // key=value

            if (i < mla_array_list_size(url.query) - 1) {
                totalLength += 1; // "&"
            }
        }
    }

    // Allocate buffer
    mla_char_t *buffer = mla_create_char_array(totalLength);

    if (buffer == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_size_t offset = 0;

    if (url.path.length == 0) {
        buffer[offset++] = '/';
    } else {
        mla_memcpy(buffer + offset, url.path.data, url.path.length);
        offset += url.path.length;
    }

    if (mla_array_list_size(url.query) > 0) {
        buffer[offset++] = '?';

        for (mla_size_t i = 0; i < mla_array_list_size(url.query); i++) {
            mla_url_query_param_t* param = mla_array_list_get_ref(url.query, i);
            mla_memcpy(buffer + offset, param->key.data, param->key.length);
            offset += param->key.length;
            buffer[offset++] = '=';
            mla_memcpy(buffer + offset, param->value.data, param->value.length);
            offset += param->value.length;
            if (i < mla_array_list_size(url.query) - 1) {
                buffer[offset++] = '&';
            }
        }
    }

    return mla_string_from_buffer_with_ownership(buffer, offset);
}


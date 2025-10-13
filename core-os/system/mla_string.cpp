//
// Created by christian on 9/10/25.
//


#include "mla_string.h"
#include "../log/mla_logging.h"

mla_char_t * mla_create_char_array(const mla_size_t p_Length) {
    return static_cast<mla_char_t*>(mla_malloc(sizeof(mla_char_t) * p_Length));
}

mla_string_t mla_string_empty() {
    return { nullptr, 0, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference_noOwner()};
}

mla_string_t mla_string(const mla_char_t *p_Data, mla_size_t p_Length) {
    return { p_Data, p_Length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference_noOwner()};
}

mla_string_t mla_string_copy(const mla_char_t *p_Data, mla_size_t p_Length) {

    if (p_Data == nullptr || p_Length == 0) {
        return mla_string_empty();
    }

    mla_char_t *newData = mla_create_char_array(p_Length);

    if (newData == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    mla_memcpy(newData, p_Data, p_Length);
    return { newData, p_Length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference(newData)};
}

mla_string_t mla_string(const mla_char_t *p_Data) {
    return { p_Data, mla_strlen(p_Data), MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference_noOwner()};
}

mla_string_t mla_string(const mla_char_t *p_Data, const mla_char_t *p_End) {
    return { p_Data, static_cast<mla_size_t>(p_End - p_Data), MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference_noOwner()};
}

mla_string_t mla_string_from_buffer_with_ownership(const mla_char_t *p_Data, mla_size_t p_Length) {
    return {p_Data, p_Length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference(p_Data)};
}

mla_string_t mla_string_from_buffer_without_ownership(mla_char_t *p_Data, mla_size_t p_Length) {
    return {p_Data, p_Length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference_noOwner()};
}

void mla_string_destroy(mla_string_t &p_String) {

    if (p_String.data == nullptr) {
        return; // Nothing to destroy
    }

    p_String.data = nullptr; // Clear the pointer
    p_String.dataOwner = mla_buffer_reference_noOwner();
    p_String.length = 0; // Reset the length
}

mla_bool_t mla_string_equals(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    if (p_String1.length != p_String2.length) {
        return false; // Lengths differ, cannot be equal
    }

    if (p_String1.data == p_String2.data) {
        return true; // Same pointer, same string
    }

    return mla_memcmp(p_String1.data, p_String2.data, p_String1.length) == 0; // Compare the actual data
}

mla_int32_t mla_string_compare(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t minLength = p_String1.length < p_String2.length ? p_String1.length : p_String2.length;

    mla_int32_t result = mla_memcmp(p_String1.data, p_String2.data, minLength);

    if (result != 0) {
        return result; // Characters differ
    }

    // All compared characters are equal, compare lengths
    if (p_String1.length < p_String2.length) {
        return -1;
    } else if (p_String1.length > p_String2.length) {
        return 1;
    }

    return 0; // Strings are equal

}

mla_int32_t mla_string_compare_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2) {

    mla_size_t minLength = p_String1.length < p_String2.length ? p_String1.length : p_String2.length;

    for (mla_size_t i = 0; i < minLength; ++i) {
        mla_char_t char1 = mla_char_toLower(p_String1.data[i]);
        mla_char_t char2 = mla_char_toLower(p_String2.data[i]);

        if (char1 < char2) {
            return -1;
        } else if (char1 > char2) {
            return 1;
        }
    }

    // All compared characters are equal, compare lengths
    if (p_String1.length < p_String2.length) {
        return -1;
    } else if (p_String1.length > p_String2.length) {
        return 1;
    }

    return 0; // Strings are equal ignoring case

}

mla_string_t mla_string_to_lower(const mla_string_t &p_String) {

    // Check if all characters are already lowercase
    mla_bool_t alreadyLower = true;
    for (mla_size_t i = 0; i < p_String.length; ++i) {
        if (p_String.data[i] != mla_char_toLower(p_String.data[i])) {
            alreadyLower = false;
            break;
        }
    }

    if (alreadyLower) {
        return p_String; // Return as-is, no conversion needed
    }

    mla_char_t *newData = mla_create_char_array(p_String.length);

    if (newData == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    for (mla_size_t i = 0; i < p_String.length; ++i) {
        newData[i] = mla_char_toLower(p_String.data[i]);
    }

    return { newData, p_String.length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference(newData)};
}

mla_string_t mla_string_to_upper(const mla_string_t &p_String) {

    // Check if all characters are already uppercase
    mla_bool_t alreadyUpper = true;
    for (mla_size_t i = 0; i < p_String.length; ++i) {
        if (p_String.data[i] != mla_char_toUpper(p_String.data[i])) {
            alreadyUpper = false;
            break;
        }
    }

    if (alreadyUpper) {
        return p_String; // Return as-is, no conversion needed
    }

    mla_char_t *newData = mla_create_char_array(p_String.length);

    if (newData == nullptr) {
        return mla_string_empty(); // Memory allocation failed
    }

    for (mla_size_t i = 0; i < p_String.length; ++i) {
        newData[i] = mla_char_toUpper(p_String.data[i]);
    }

    return { newData, p_String.length, MLA_STRING_MEMORY_LAYOUT_BUFFER, mla_buffer_reference(newData)};
}


mla_bool_t mla_string_is_empty(const mla_string_t &p_String) {
    return p_String.length == 0;
}

mla_bool_t mla_string_equals_ignore_case(const mla_string_t &p_String1, const mla_string_t &p_String2) {
    if (p_String1.length != p_String2.length) {
        return false; // Lengths differ, cannot be equal
    }
    for (mla_size_t i = 0; i < p_String1.length; ++i) {
        if (mla_char_toLower(p_String1.data[i]) != mla_char_toLower(p_String2.data[i])) {
            return false; // Characters differ, not equal
        }
    }
    return true; // All characters are equal ignoring case
}

mla_bool_t mla_string_contains(const mla_string_t &p_String, const mla_string_t &p_Substring) {
    if (p_Substring.length > p_String.length) {
        return false; // Substring cannot be longer than the string
    }

    if (p_String.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING && p_Substring.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {
        // If both strings are C-style strings, use strstr for substring search
        // because it is more efficient and optimized for this purpose
        return mla_strstr(p_String.data, p_Substring.data) != nullptr; // Use strstr for substring search
    }

    for (mla_size_t i = 0; i <= p_String.length - p_Substring.length; ++i) {

        if (mla_memcmp(p_String.data + i, p_Substring.data, p_Substring.length) == 0)
            return true;

    }
    return false; // Substring not found
}

mla_bool_t mla_string_starts_with(const mla_string_t &p_String, const mla_string_t &p_Prefix) {
    if (p_Prefix.length > p_String.length) {
        return false; // Prefix cannot be longer than the string
    }

    return mla_memcmp(p_String.data, p_Prefix.data, p_Prefix.length) == 0; // Compare the prefix

}

mla_bool_t mla_string_ends_with(const mla_string_t &p_String, const mla_string_t &p_Suffix) {
    if (p_Suffix.length > p_String.length) {
        return false; // Suffix cannot be longer than the string
    }

    return mla_memcmp(p_String.data + p_String.length - p_Suffix.length, p_Suffix.data, p_Suffix.length) == 0; // Compare the suffix

}

mla_int32_t mla_string_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring) {

    if (p_Substring.length > p_String.length) {
        return -1; // Substring cannot be longer than the string
    }

    if (p_String.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING && p_Substring.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {
        const mla_char_t *found = mla_strstr(p_String.data, p_Substring.data);

        if (found) {
            return static_cast<mla_int32_t>(found - p_String.data);
        } else{
            return -1; // Substring not found
        }

    }

    // Manual search for the substring
    for (mla_size_t i = 0; i <= p_String.length - p_Substring.length; ++i) {

        if (mla_memcmp(p_String.data + i, p_Substring.data, p_Substring.length) == 0)
            return static_cast<mla_int32_t>(i);

    }
    return -1; // Substring not found
}


mla_int32_t mla_string_last_index_of(const mla_string_t &p_String, const mla_string_t &p_Substring) {
    if (p_Substring.length > p_String.length) {
        return -1; // Substring cannot be longer than the string
    }
    for (mla_int32_t i = p_String.length - p_Substring.length; i != -1; --i) {

        if (mla_memcmp(p_String.data + i, p_Substring.data, p_Substring.length) == 0) {
            return i; // Found the substring
        }

    }
    return -1; // Substring not found
}

mla_string_t mla_string_substr(const mla_string_t &p_String, mla_size_t p_Start, mla_size_t p_End) {

    // For an substring we dont need to copy any data we can just play with pointers
    if (p_Start >= p_String.length || p_End >= p_String.length || p_Start > p_End) {
        return mla_string_empty(); // Invalid range
    }

    return {
        p_String.data + p_Start,
        p_End - p_Start + 1,
        MLA_STRING_MEMORY_LAYOUT_SUB_STRING,
        p_String.dataOwner
    };
}

mla_array_list_t<mla_string_t, mla_string_initializer> mla_string_split(const mla_string_t &p_String, const mla_string_t &p_Delimiter) {

    mla_array_list_t<mla_string_t, mla_string_initializer> result = mla_array_list_empty<mla_string_t, mla_string_initializer>();

    if (p_Delimiter.length == 0) {
        // If the delimiter is empty, return the original string as the only element
        mla_array_list_add(result, p_String);
        return result;
    }

    // Split the string
    mla_size_t start = 0;
    mla_int32_t delimiterIndex = 0;

    while (start < p_String.length) {
        // Create a substring from the current position
        mla_string_t searchString = mla_string_substr(p_String, start, p_String.length - 1);

        // Find the next delimiter
        delimiterIndex = mla_string_index_of(searchString, p_Delimiter);

        if (delimiterIndex == -1) {
            // No more delimiters found, add the rest of the string
            mla_string_t part = mla_string_substr(p_String, start, p_String.length - 1);
            mla_array_list_add(result, part);
            break;
        }

        // Add the substring before the delimiter
        mla_size_t end = start + delimiterIndex - 1;
        mla_string_t part = mla_string_substr(p_String, start, end);
        mla_array_list_add(result, part);

        // Move past the delimiter
        start = start + delimiterIndex + p_Delimiter.length;
    }

    return result;
}

mla_string_t mla_string_trim(const mla_string_t &p_String) {

    mla_size_t start = 0;
    mla_size_t end = p_String.length;

    // Trim leading whitespace
    while (start < end && mla_char_is_whitespace(p_String.data[start])) {
        ++start;
    }

    // Trim trailing whitespace
    while (end > start && mla_char_is_whitespace(p_String.data[end - 1])) {
        --end;
    }

    if (start == 0 && end == p_String.length) {
        return p_String; // No trimming needed
    }

    return {
        p_String.data + start,
        end - start,
        MLA_STRING_MEMORY_LAYOUT_SUB_STRING,
        p_String.dataOwner
    };
}

mla_bool_t mla_string_change_memory_layout(mla_string_t &p_String, mla_string_memory_layout_t p_NewLayout) {

    if (p_String.memoryLayout == p_NewLayout) {
        return true; // No change needed
    }

    if (p_NewLayout == MLA_STRING_MEMORY_LAYOUT_SUB_STRING) {
        // Substrings are views into other strings, so we cannot convert to this layout directly
        mla_error("Cannot convert to substring layout directly.");
        return false;
    }

    if (p_NewLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {
        // Convert to C-style string
        mla_size_t newLength = p_String.length + 1; // +1 for null terminator
        mla_char_t *newData = mla_create_char_array(newLength);

        if (newData == nullptr) {
            return false; // Memory allocation failed
        }

        mla_memcpy(newData, p_String.data, p_String.length);
        newData[p_String.length] = '\0'; // Null-terminate the string
        p_String = { newData, newLength - 1, MLA_STRING_MEMORY_LAYOUT_C_STRING, mla_buffer_reference(newData)}; // Update the string instance
        return true;
    } else {
        // Convert to buffer-based string

        p_String.memoryLayout = MLA_STRING_MEMORY_LAYOUT_BUFFER;
        return true;
    }
}

mla_c_string_t mla_string_to_cString(mla_string_t &p_String, mla_bool_t p_ForceCopy) {

    if (!p_ForceCopy) {

        // convert the instance into a C-style string
        // because we need now to copy any way and this will speed the data handling later
        // because c string are more efficient for string operations
        mla_string_change_memory_layout(p_String, MLA_STRING_MEMORY_LAYOUT_C_STRING);
        return {p_String.data, false};
    }


    mla_char_t *cString = mla_create_char_array(p_String.length + 1); // +1 for null terminator

    if (cString == nullptr) {
        return {nullptr, false}; // Memory allocation failed
    }

    mla_memcpy(cString, p_String.data, p_String.length);
    cString[p_String.length] = '\0'; // Null-terminate the string
    return { cString, true};
}

mla_c_string_t mla_string_to_cString(const mla_string_t &p_String) {

    if (p_String.memoryLayout == MLA_STRING_MEMORY_LAYOUT_C_STRING) {
        return {p_String.data, false}; // Already a C-style string, no need to copy
    }

    mla_char_t *cString = mla_create_char_array(p_String.length + 1); // +1 for null terminator

    if (cString == nullptr) {
        return {nullptr, false}; // Memory allocation failed
    }

    mla_memcpy(cString, p_String.data, p_String.length);
    cString[p_String.length] = '\0'; // Null-terminate the string
    return { cString, true};
}
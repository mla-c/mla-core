//
// Created by chris on 8/2/2025.
//

#ifndef COREOS_MLA_STRING_TEST_H
#define COREOS_MLA_STRING_TEST_H

#include "../core-os/memory/mla_memory_hook.h"
#include "../core-os/system/mla_string.h"
#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os-test-support/mla_benchmark_executor.h"

void SizeOfTest() {
    mla_size_t size = sizeof(mla_pointer_t);
    assert_equal((mla_test_int32_t)sizeof(mla_string_t), (mla_test_int32_t)size * 3, "Size of mla_string_t should be 24 bytes");
}

void ContainsCLayoutTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void ContainsBufferLayoutTest() {
    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be C layout");
    assert_true(mla_string_contains(mla_str, mla_string("World")), "MlaString should contain 'World'");
    assert_false(mla_string_contains(mla_str, mla_string("world")), "MlaString should not contain 'world'");
}

void StartsWithTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_starts_with(mla_str, mla_string("Hello")), "MlaString should start with 'Hello'");
    assert_false(mla_string_starts_with(mla_str, mla_string("World")), "MlaString should not start with 'World'");
}

void LengthTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.length, (mla_uint32_t)13, "MlaString length should be 13");
    mla_string_t empty_mla_str = mla_string("");
    assert_equal(empty_mla_str.length, (mla_uint32_t)0, "Length of empty MlaString should be 0");
}

void EndsWithTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_true(mla_string_ends_with(mla_str, mla_string("World!")), "MlaString should end with 'World!'");
    assert_false(mla_string_ends_with(mla_str, mla_string("Hello")), "MlaString should not end with 'Hello'");
}

void EqualsTest() {
    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("Hello, World!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals(mla_str1, mla_str2), "MlaString should be equal");
    assert_false(mla_string_equals(mla_str1, mla_str3), "MlaString should not be equal");
}

void EqualsIgnoreCaseTest() {
    mla_string_t mla_str1 = mla_string("Hello, World!");
    mla_string_t mla_str2 = mla_string("hello, wOrld!");
    mla_string_t mla_str3 = mla_string("Goodbye, World!");

    assert_true(mla_string_equals_ignore_case(mla_str1, mla_str2), "MlaString should be equal ignoring case");
    assert_false(mla_string_equals_ignore_case(mla_str1, mla_str3), "MlaString should not be equal ignoring case");
}

void IndexOfCLayoutTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_C_STRING, "MlaString should be C layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7, "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString index of 'world' should be -1 (not found)");
}

void IndexOfBufferLayoutTest() {
    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    assert_equal(mla_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_BUFFER, "MlaString should be buffer layout");
    assert_equal(mla_string_index_of(mla_str, mla_string("World")), (mla_int32_t)7, "MlaString index of 'World' should be 7");
    assert_equal(mla_string_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString index of 'world' should be -1 (not found)");
}

void LastIndexOfTest() {
    mla_string_t mla_str = mla_string("Hello, World! Hello, Universe!");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("Hello")), (mla_int32_t)14, "MlaString last index of 'Hello' should be 19");
    assert_equal(mla_string_last_index_of(mla_str, mla_string("world")), (mla_int32_t)-1, "MlaString last index of 'world' should be -1 (not found)");
}

void ToCStringTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, true);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");

    if (mla_c_str.c_str != nullptr) {
        assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
        assert_true(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");
        mla_free(const_cast<mla_char_t*>(mla_c_str.c_str)); // Clean up allocated memory
    } else {
        assert_fail("MlaString C-string conversion failed");
    }
}

void ToCString_No_Force_CopyTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str.isOwner, "MlaString C-string should not be owned by the caller");
}

void ToCStringFromBufferTest() {
    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, true);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");

    if (mla_c_str.c_str != nullptr) {
        assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
        assert_true(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");
        mla_free(const_cast<mla_char_t*>(mla_c_str.c_str)); // Clean up allocated memory
    } else {
        assert_fail("MlaString C-string conversion failed");
    }
}

void ToCStringFromBuffer_No_Force_CopyTest() {
    mla_string_t mla_str = mla_string("Hello, World!", 13); // Explicitly set length for buffer layout
    mla_c_string_t mla_c_str = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str.isOwner, "MlaString C-string should be owned by the caller");

    mla_c_string_t mla_c_str2 = mla_string_to_cString(mla_str, false);
    assert_true(mla_c_str2.c_str != nullptr, "MlaString C-string should not be null");
    assert_equal(mla_strlen(mla_c_str2.c_str), (mla_uint32_t)13, "MlaString C-string length should be 13");
    assert_false(mla_c_str2.isOwner, "MlaString C-string should not be owned by the caller");
}

void AccessCharTest() {
    mla_string_t mla_str = mla_string("Hello, World!");
    assert_equal(mla_str.data[0], 'H', "First character of MlaString should be 'H'");
    assert_equal(mla_str.data[7], 'W', "Eighth character of MlaString should be 'W'");
    assert_equal(mla_str.data[12], '!', "Last character of MlaString should be '!'");
}

void ConcatTest() {
    mla_string_t mla_str1 = mla_string("Hello, ");
    mla_string_t mla_str2 = mla_string("World!");
    mla_string_t mla_result = mla_string_concat(mla_str1, mla_str2);

    assert_equal(mla_result.length, (mla_uint32_t)13, "MlaString concatenated length should be 13");
    assert_true(mla_string_equals(mla_result, mla_string("Hello, World!")), "MlaString concatenated should equal 'Hello, World!'");

    mla_string_destroy(mla_result);
}

static mla_pointer_t AutoMemoryManagementTest_last_pointer;

mla_bool_t AutoMemoryManagementTest_Malloc(mla_size_t size, mla_pointer_t*out_ptr) {
    (void) size;
    (void) out_ptr;
    return false;
}

mla_bool_t AutoMemoryManagementTest_Free(mla_pointer_t ptr) {
    AutoMemoryManagementTest_last_pointer = ptr;
    return false;
}

void AutoMemoryManagementTest() {
    mla_buffer_t *unsafePointer = nullptr;

    mla_memory_hook_t hook = mla_memory_hook_install(AutoMemoryManagementTest_Malloc, AutoMemoryManagementTest_Free); {
        mla_string_t datacopy = mla_string_empty(); {
            mla_string_t data = mla_string_concat(mla_string("Hello, "), mla_string("World!"), mla_string(" This is a test of concatenation."));
            assert_equal(data.length, (mla_uint32_t)46, "Concatenated string length should be 58");

            if (data.dataOwner.buffer != nullptr) {
                assert_equal(data.dataOwner.buffer->refCount, (mla_uint32_t)1, "Reference count should be 1 after concatenation");
                datacopy = data;
                assert_equal(data.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after copying");
                assert_equal(datacopy.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after copying");
                assert_equal(datacopy.dataOwner.buffer->data, data.dataOwner.buffer->data, "Copied string data should match original");
            } else {
                assert_fail("Data buffer should not be null after concatenation");
            }

            data = mla_string_empty(); // Clear the original string
        }

        if (datacopy.dataOwner.buffer != nullptr) {
            assert_equal(datacopy.dataOwner.buffer->refCount, (mla_uint32_t)1, "Reference count should be 1 after clearing the original string");

            assert_equal(datacopy.length, (mla_uint32_t)46, "Copied string length should still be 58");
            assert_true(mla_string_equals(datacopy, mla_string("Hello, World! This is a test of concatenation.")),
                        "Copied string should equal 'Hello, World! This is a test of concatenation.'");
        } else {
            assert_fail("Data buffer should not be null after clearing");
        }

        AutoMemoryManagementTest_last_pointer = nullptr;
        unsafePointer = datacopy.dataOwner.buffer;
    }
    // Check ich the buffer was released
    assert_equal(AutoMemoryManagementTest_last_pointer, unsafePointer, "Last pointer should match the unsafe pointer");

    mla_memory_hook_uninstall(hook);
    unsafePointer = nullptr; // Clear the unsafe pointer
}

void SubStringTest() {
    mla_string_t mla_str = mla_string_concat(mla_string("Hello, "), mla_string("World!"));
    mla_string_t sub_str = mla_string_substr(mla_str, 7, 11); // "World"

    // Check memory managemant
    if (sub_str.dataOwner.buffer != nullptr) {
        assert_equal(sub_str.dataOwner.buffer->refCount, (mla_uint32_t)2, "Reference count should be 2 after creating substring");
    } else {
        assert_fail("Data buffer should not be null after concatenation");
    }

    assert_equal(sub_str.memoryLayout, MLA_STRING_MEMORY_LAYOUT_SUB_STRING, "Substring should have SUB_STRING layout");
    assert_equal(sub_str.length, (mla_uint32_t)5, "Substring length should be 5");
    assert_true(mla_string_equals(sub_str, mla_string("World")), "Substring should equal 'World'");
}

void MultiByteCharHandlingTest() {
    mla_string_t mla_str = mla_string("€ 100");
    assert_equal(mla_str.length, (mla_uint32_t)7, "MlaString length should be 7 bytes");

    assert_equal(mla_string_multi_byte_char_count(mla_str), (mla_uint32_t)5, "MlaString should have 4 multi-byte characters");

    mla_multi_byte_char_t mb_char = mla_string_multi_byte_char_at(mla_str, 0);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)0xE2, "First byte of Euro sign should be 0xE2");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x82, "Second byte of Euro sign should be 0x82");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0xAC, "Third byte of Euro sign should be 0xAC");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Fourth byte of Euro sign should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 1);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)' ', "First byte of space should be ' '");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 2);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'1', "First byte of space should be '1'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 3);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'0', "First byte of space should be '0'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 4);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)'0', "First byte of space should be '0'");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");

    mb_char = mla_string_multi_byte_char_at(mla_str, 5);
    assert_equal(mb_char.bytes[0], (mla_char_t)(mla_uint8_t)0x00, "First byte of space should be null terminator");
    assert_equal(mb_char.bytes[1], (mla_char_t)(mla_uint8_t)0x00, "Second byte of space should be null terminator");
    assert_equal(mb_char.bytes[2], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
    assert_equal(mb_char.bytes[3], (mla_char_t)(mla_uint8_t)0x00, "Third byte of space should be null terminator");
}

void ToUtf16AndFromUtf16Test() {
    mla_string_t baseString = mla_string("€ 100");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)5, "UTF-16 buffer should have 5 characters");

    // Euro sign is U+20AC, which is 0x20AC in UTF-16
    if (utf16Buffer.data != nullptr) {
        assert_equal(utf16Buffer.data[0], (mla_utf_16_char_t)0x20AC, "First UTF-16 character should be Euro sign");
        assert_equal(utf16Buffer.data[1], (mla_utf_16_char_t)' ', "Second UTF-16 character should be space");
        assert_equal(utf16Buffer.data[2], (mla_utf_16_char_t)'1', "Third UTF-16 character should be '1'");
        assert_equal(utf16Buffer.data[3], (mla_utf_16_char_t)'0', "Fourth UTF-16 character should be '0'");
        assert_equal(utf16Buffer.data[4], (mla_utf_16_char_t)'0', "Fifth UTF-16 character should be '0'");
        assert_equal(utf16Buffer.data[5], (mla_utf_16_char_t)0x00, "Six UTF-16 character should be null terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t newBaseString = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, newBaseString, "New MlaString from UTF-16 buffer should equal original");

    mla_string_utf16_buffer_destroy(utf16Buffer);
}

void ToUtf32AndFromUtf32Test() {
    mla_string_t baseString = mla_string("€ 100");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)5, "UTF-32 buffer should have 5 characters");

    // Euro sign is U+20AC, which is 0x20AC in UTF-32

    if (utf32Buffer.data != nullptr) {
        assert_equal(utf32Buffer.data[0], (mla_utf_32_char_t)0x000020AC, "First UTF-32 character should be Euro sign");
        assert_equal(utf32Buffer.data[1], (mla_utf_32_char_t)' ', "Second UTF-32 character should be space");
        assert_equal(utf32Buffer.data[2], (mla_utf_32_char_t)'1', "Third UTF-32 character should be '1'");
        assert_equal(utf32Buffer.data[3], (mla_utf_32_char_t)'0', "Fourth UTF-32 character should be '0'");
        assert_equal(utf32Buffer.data[4], (mla_utf_32_char_t)'0', "Fifth UTF-32 character should be '0'");
        assert_equal(utf32Buffer.data[5], (mla_utf_32_char_t)0x00000000, "Sixth UTF-32 character should be null terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }


    mla_string_t newBaseString = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, newBaseString, "New MlaString from UTF-32 buffer should equal original");
    mla_string_utf32_buffer_destroy(utf32Buffer);
}

void ToUtf16AndFromUtf16_EmptyTest() {
    mla_string_t baseString = mla_string("");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)0, "UTF-16 empty: charCount should be 0");
    assert_null(utf16Buffer.data, "UTF-16 empty: data shoud be null");
    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-16 empty round trip should match");
    mla_string_utf16_buffer_destroy(utf16Buffer);
}

void ToUtf16AndFromUtf16_AsciiTest() {
    mla_string_t baseString = mla_string("ABC");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)3, "UTF-16 ASCII: charCount should be 3");

    if (utf16Buffer.data != nullptr) {
        assert_equal(utf16Buffer.data[0], (mla_utf_16_char_t)'A', "UTF-16 ASCII: data[0]=='A'");
        assert_equal(utf16Buffer.data[1], (mla_utf_16_char_t)'B', "UTF-16 ASCII: data[1]=='B'");
        assert_equal(utf16Buffer.data[2], (mla_utf_16_char_t)'C', "UTF-16 ASCII: data[2]=='C'");
        assert_equal(utf16Buffer.data[3], (mla_utf_16_char_t)0x00, "UTF-16 ASCII: null terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-16 ASCII round trip should match");
    mla_string_utf16_buffer_destroy(utf16Buffer);
}

void ToUtf16AndFromUtf16_MixedBmpTest() {
    // Euro (U+20AC) Omega (U+03A9) Eszett (U+00DF)
    mla_string_t baseString = mla_string("€Ωß");
    mla_string_utf16_buffer_t utf16Buffer = mla_string_to_utf16_buffer(baseString);
    assert_equal(utf16Buffer.charCount, (mla_uint32_t)3, "UTF-16 Mixed BMP: charCount should be 3");

    if (utf16Buffer.data != nullptr) {
        assert_equal(utf16Buffer.data[0], (mla_utf_16_char_t)0x20AC, "UTF-16 Mixed BMP: Euro");
        assert_equal(utf16Buffer.data[1], (mla_utf_16_char_t)0x03A9, "UTF-16 Mixed BMP: Omega");
        assert_equal(utf16Buffer.data[2], (mla_utf_16_char_t)0x00DF, "UTF-16 Mixed BMP: Eszett");
        assert_equal(utf16Buffer.data[3], (mla_utf_16_char_t)0x00, "UTF-16 Mixed BMP: terminator");
    } else {
        assert_fail("UTF-16 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf16_buffer(utf16Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-16 Mixed BMP round trip should match");
    mla_string_utf16_buffer_destroy(utf16Buffer);
}

void ToUtf32AndFromUtf32_EmptyTest() {
    mla_string_t baseString = mla_string("");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)0, "UTF-32 empty: charCount should be 0");
    assert_null(utf32Buffer.data, "UTF-32 empty: data should be null");
    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-32 empty round trip should match");
    mla_string_utf32_buffer_destroy(utf32Buffer);
}

void ToUtf32AndFromUtf32_AsciiTest() {
    mla_string_t baseString = mla_string("ABC");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)3, "UTF-32 ASCII: charCount should be 3");

    if (utf32Buffer.data != nullptr) {
        assert_equal(utf32Buffer.data[0], (mla_utf_32_char_t)'A', "UTF-32 ASCII: data[0]=='A'");
        assert_equal(utf32Buffer.data[1], (mla_utf_32_char_t)'B', "UTF-32 ASCII: data[1]=='B'");
        assert_equal(utf32Buffer.data[2], (mla_utf_32_char_t)'C', "UTF-32 ASCII: data[2]=='C'");
        assert_equal(utf32Buffer.data[3], (mla_utf_32_char_t)0x00000000, "UTF-32 ASCII: terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-32 ASCII round trip should match");
    mla_string_utf32_buffer_destroy(utf32Buffer);
}

void ToUtf32AndFromUtf32_MixedBmpTest() {
    mla_string_t baseString = mla_string("€Ωß");
    mla_string_utf32_buffer_t utf32Buffer = mla_string_to_utf32_buffer(baseString);
    assert_equal(utf32Buffer.charCount, (mla_uint32_t)3, "UTF-32 Mixed BMP: charCount should be 3");

    if (utf32Buffer.data != nullptr) {
        assert_equal(utf32Buffer.data[0], (mla_utf_32_char_t)0x000020AC, "UTF-32 Mixed BMP: Euro");
        assert_equal(utf32Buffer.data[1], (mla_utf_32_char_t)0x000003A9, "UTF-32 Mixed BMP: Omega");
        assert_equal(utf32Buffer.data[2], (mla_utf_32_char_t)0x000000DF, "UTF-32 Mixed BMP: Eszett");
        assert_equal(utf32Buffer.data[3], (mla_utf_32_char_t)0x00000000, "UTF-32 Mixed BMP: terminator");
    } else {
        assert_fail("UTF-32 buffer is empty");
    }

    mla_string_t roundTrip = mla_string_from_utf32_buffer(utf32Buffer);
    assert_struct_equal(mla_string_t, baseString, roundTrip, "UTF-32 Mixed BMP round trip should match");
    mla_string_utf32_buffer_destroy(utf32Buffer);
}

void StringFromInt8Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int8(42);
    assert_true(mla_string_equals(str, mla_string("42")), "int8(42) should equal '42'");
    mla_string_destroy(str);

    // Test negative value
    str = mla_string_from_int8(-42);
    assert_true(mla_string_equals(str, mla_string("-42")), "int8(-42) should equal '-42'");
    mla_string_destroy(str);

    // Test min value
    str = mla_string_from_int8(-128);
    assert_true(mla_string_equals(str, mla_string("-128")), "int8 min should equal '-128'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_int8(127);
    assert_true(mla_string_equals(str, mla_string("127")), "int8 max should equal '127'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_int8(0);
    assert_true(mla_string_equals(str, mla_string("0")), "int8(0) should equal '0'");
    mla_string_destroy(str);
}

void StringFromUInt8Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint8(42);
    assert_true(mla_string_equals(str, mla_string("42")), "uint8(42) should equal '42'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_uint8(255);
    assert_true(mla_string_equals(str, mla_string("255")), "uint8 max should equal '255'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_uint8(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint8(0) should equal '0'");
    mla_string_destroy(str);
}

void StringFromInt16Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int16(1234);
    assert_true(mla_string_equals(str, mla_string("1234")), "int16(1234) should equal '1234'");
    mla_string_destroy(str);

    // Test negative value
    str = mla_string_from_int16(-1234);
    assert_true(mla_string_equals(str, mla_string("-1234")), "int16(-1234) should equal '-1234'");
    mla_string_destroy(str);

    // Test min value
    str = mla_string_from_int16(-32768);
    assert_true(mla_string_equals(str, mla_string("-32768")), "int16 min should equal '-32768'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_int16(32767);
    assert_true(mla_string_equals(str, mla_string("32767")), "int16 max should equal '32767'");
    mla_string_destroy(str);
}

void StringFromUInt16Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint16(1234);
    assert_true(mla_string_equals(str, mla_string("1234")), "uint16(1234) should equal '1234'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_uint16(65535);
    assert_true(mla_string_equals(str, mla_string("65535")), "uint16 max should equal '65535'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_uint16(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint16(0) should equal '0'");
    mla_string_destroy(str);
}

void StringFromInt32Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int32(123456);
    assert_true(mla_string_equals(str, mla_string("123456")), "int32(123456) should equal '123456'");
    mla_string_destroy(str);

    // Test negative value
    str = mla_string_from_int32(-123456);
    assert_true(mla_string_equals(str, mla_string("-123456")), "int32(-123456) should equal '-123456'");
    mla_string_destroy(str);

    // Test min value
    str = mla_string_from_int32(mla_int32_min);
    assert_true(mla_string_equals(str, mla_string("-2147483648")), "int32 min should equal '-2147483648'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_int32(mla_int32_max);
    assert_true(mla_string_equals(str, mla_string("2147483647")), "int32 max should equal '2147483647'");
    mla_string_destroy(str);
}

void StringFromUInt32Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint32(123456);
    assert_true(mla_string_equals(str, mla_string("123456")), "uint32(123456) should equal '123456'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_uint32(mla_uint32_max);
    assert_true(mla_string_equals(str, mla_string("4294967295")), "uint32 max should equal '4294967295'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_uint32(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint32(0) should equal '0'");
    mla_string_destroy(str);
}

void StringFromInt64Test() {
    // Test positive value
    mla_string_t str = mla_string_from_int64(123456789012LL);
    assert_true(mla_string_equals(str, mla_string("123456789012")), "int64(123456789012) should equal '123456789012'");
    mla_string_destroy(str);

    // Test negative value
    str = mla_string_from_int64(-123456789012LL);
    assert_true(mla_string_equals(str, mla_string("-123456789012")), "int64(-123456789012) should equal '-123456789012'");
    mla_string_destroy(str);

    // Test min value
    str = mla_string_from_int64(mla_int64_min);
    assert_true(mla_string_equals(str, mla_string("-9223372036854775808")), "int64 min should equal '-9223372036854775808'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_int64(mla_int64_max);
    assert_true(mla_string_equals(str, mla_string("9223372036854775807")), "int64 max should equal '9223372036854775807'");
    mla_string_destroy(str);
}

void StringFromUInt64Test() {
    // Test typical value
    mla_string_t str = mla_string_from_uint64(123456789012ULL);
    assert_true(mla_string_equals(str, mla_string("123456789012")), "uint64(123456789012) should equal '123456789012'");
    mla_string_destroy(str);

    // Test max value
    str = mla_string_from_uint64(mla_uint64_max);
    assert_true(mla_string_equals(str, mla_string("18446744073709551615")), "uint64 max should equal '18446744073709551615'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_uint64(0);
    assert_true(mla_string_equals(str, mla_string("0")), "uint64(0) should equal '0'");
    mla_string_destroy(str);
}

void StringFromFloatTest() {
    // Test with 2 decimal places
    mla_string_t str = mla_string_from_float(3.14f, 2);
    assert_true(mla_string_equals(str, mla_string("3.14")), "float(3.14, 2) should equal '3.14'");
    mla_string_destroy(str);

    // Test negative with 3 decimal places
    str = mla_string_from_float(-123.456f, 3);
    assert_true(mla_string_equals(str, mla_string("-123.456")), "float(-123.456, 3) should equal '-123.456'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_float(0.0f, 1);
    assert_true(mla_string_equals(str, mla_string("0.0")), "float(0.0, 1) should equal '0.0'");
    mla_string_destroy(str);

    // Test with 0 decimal places
    str = mla_string_from_float(42.789f, 0);
    assert_true(mla_string_equals(str, mla_string("43")), "float(42.789, 0) should round to '43'");
    mla_string_destroy(str);
}

void StringFromDoubleTest() {
    // Test with 2 decimal places
    mla_string_t str = mla_string_from_double(3.14159265358979, 2);
    assert_true(mla_string_equals(str, mla_string("3.14")), "double(pi, 2) should equal '3.14'");
    mla_string_destroy(str);

    // Test negative with 5 decimal places
    str = mla_string_from_double(-123.456789, 5);
    assert_true(mla_string_equals(str, mla_string("-123.45679")), "double(-123.456789, 5) should equal '-123.45679'");
    mla_string_destroy(str);

    // Test zero
    str = mla_string_from_double(0.0, 3);
    assert_true(mla_string_equals(str, mla_string("0.000")), "double(0.0, 3) should equal '0.000'");
    mla_string_destroy(str);

    // Test with 0 decimal places
    str = mla_string_from_double(42.789, 0);
    assert_true(mla_string_equals(str, mla_string("43")), "double(42.789, 0) should round to '43'");
    mla_string_destroy(str);
}

void StringFromBoolTest() {
    // Test true
    mla_string_t str = mla_string_from_bool(true);
    assert_true(mla_string_equals(str, mla_string("true")), "bool(true) should equal 'true'");

    // Test false
    str = mla_string_from_bool(false);
    assert_true(mla_string_equals(str, mla_string("false")), "bool(false) should equal 'false'");

    // Test with 1 (should be true)
    str = mla_string_from_bool(1);
    assert_true(mla_string_equals(str, mla_string("true")), "bool(1) should equal 'true'");

    // Test with 0 (should be false)
    str = mla_string_from_bool(0);
    assert_true(mla_string_equals(str, mla_string("false")), "bool(0) should equal 'false'");
}


void RegisterStringTests(mla_test_executor_t &p_TestExecutor) {
    mla_test_t test = mla_test("SizeOf", test_category, SizeOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsCLayout", test_category, ContainsCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ContainsBufferLayout", test_category, ContainsBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("StartsWith", test_category, StartsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Length", test_category, LengthTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EndsWith", test_category, EndsWithTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Equals", test_category, EqualsTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("EqualsIgnoreCase", test_category, EqualsIgnoreCaseTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOf", test_category, IndexOfCLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("IndexOfBufferLayout", test_category, IndexOfBufferLayoutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("LastIndexOf", test_category, LastIndexOfTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCString", test_category, ToCStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCString_No_Force_Copy", test_category, ToCString_No_Force_CopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer", test_category, ToCStringFromBufferTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToCStringFromBuffer_No_Force_Copy", test_category, ToCStringFromBuffer_No_Force_CopyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AccessChar", test_category, AccessCharTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("Concat", test_category, ConcatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("AutoMemoryManagement", test_category, AutoMemoryManagementTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("SubString", test_category, SubStringTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MultiByteCharHandling", test_category, MultiByteCharHandlingTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16", test_category, ToUtf16AndFromUtf16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32", test_category, ToUtf32AndFromUtf32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_Empty", test_category, ToUtf16AndFromUtf16_EmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_Ascii", test_category, ToUtf16AndFromUtf16_AsciiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf16AndFromUtf16_MixedBmp", test_category, ToUtf16AndFromUtf16_MixedBmpTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_Empty", test_category, ToUtf32AndFromUtf32_EmptyTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_Ascii", test_category, ToUtf32AndFromUtf32_AsciiTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ToUtf32AndFromUtf32_MixedBmp", test_category, ToUtf32AndFromUtf32_MixedBmpTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt8", test_category, StringFromInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt8", test_category, StringFromUInt8Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt16", test_category, StringFromInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt16", test_category, StringFromUInt16Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt32", test_category, StringFromInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt32", test_category, StringFromUInt32Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromInt64", test_category, StringFromInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromUInt64", test_category, StringFromUInt64Test);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromFloat", test_category, StringFromFloatTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromDouble", test_category, StringFromDoubleTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("FromBool", test_category, StringFromBoolTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

void StringConcatBenchmark() {
    mla_string_t str1 = mla_string("Hello, ");
    mla_string_t str2 = mla_string("World!");
    mla_string_t str3 = mla_string(" This is a test of concatenation.");

    mla_string_t result = mla_string_concat(str1, str2, str3);

    mla_test_int32_t length = result.length;
    (void) length; // Prevent unused variable warning

    mla_string_destroy(result);
}

void StringFromInt8Benchmark() {
    mla_int8_t value = 42;
    mla_string_t str = mla_string_from_int8(value);
    mla_string_destroy(str);
}

void StringFromInt16Benchmark() {
    mla_int16_t value = 1234;
    mla_string_t str = mla_string_from_int16(value);
    mla_string_destroy(str);
}

void StringFromInt32Benchmark() {
    mla_int32_t value = 123456;
    mla_string_t str = mla_string_from_int32(value);
    mla_string_destroy(str);
}

void StringFromInt64Benchmark() {
    mla_int64_t value = 123456789012LL;
    mla_string_t str = mla_string_from_int64(value);
    mla_string_destroy(str);
}

void StringFromUInt8Benchmark() {
    mla_uint8_t value = 42;
    mla_string_t str = mla_string_from_uint8(value);
    mla_string_destroy(str);
}

void StringFromUInt16Benchmark() {
    mla_uint16_t value = 1234;
    mla_string_t str = mla_string_from_uint16(value);
    mla_string_destroy(str);
}

void StringFromUInt32Benchmark() {
    mla_uint32_t value = 123456;
    mla_string_t str = mla_string_from_uint32(value);
    mla_string_destroy(str);
}

void StringFromUInt64Benchmark() {
    mla_uint64_t value = 123456789012ULL;
    mla_string_t str = mla_string_from_uint64(value);
    mla_string_destroy(str);
}

void StringFromFloatBenchmark() {
    mla_float_t value = 3.14159f;
    mla_string_t str = mla_string_from_float(value, 5);
    mla_string_destroy(str);
}

void StringFromDoubleBenchmark() {
    mla_double_t value = 3.14159265358979;
    mla_string_t str = mla_string_from_double(value, 10);
    mla_string_destroy(str);
}

void StringFromBoolBenchmark() {
    mla_bool_t value = true;
    mla_string_t str = mla_string_from_bool(value);
    mla_string_destroy(str);
}

void StringFromUtf16BufferBenchmark() {
    mla_utf_16_char_t data[] = {0x20AC, ' ', '1', '0', '0', 0x00};
    mla_string_utf16_buffer_t buffer = {data, 5};
    mla_string_t str = mla_string_from_utf16_buffer(buffer);
    mla_string_destroy(str);
}

void StringFromUtf32BufferBenchmark() {
    mla_utf_32_char_t data[] = {0x000020AC, ' ', '1', '0', '0', 0x00000000};
    mla_string_utf32_buffer_t buffer = {data, 5};
    mla_string_t str = mla_string_from_utf32_buffer(buffer);
    mla_string_destroy(str);
}


void StringContains_Buffer_LayoutBenchmark() {
    const mla_test_char_t *data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = (mla_test_int32_t) strlen(data); // Length of the string

    mla_string_t str = mla_string(data, length);
    mla_string_t subString = mla_string("for");

    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringContains_C_LayoutBenchmark() {
    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string("for");


    if (!mla_string_contains(str, subString)) {
        static_assert(true, "Substring not found in MlaStringContainsBenchmark");
    }
}

void StringIndexOf_Buffer_LayoutBenchmark() {
    const mla_test_char_t *data = "Hello, World! This is a test string for benchmarking.";
    mla_test_int32_t length = (mla_test_int32_t) strlen(data); // Length of the string

    mla_string_t str = mla_string(data, length);
    mla_string_t subString = mla_string("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }
}

void StringIndexOf_C_LayoutBenchmark() {
    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_string_t subString = mla_string("for");

    mla_test_int32_t index = mla_string_index_of(str, subString);
    if (index != 38) {
        static_assert(true, "Index of substring not found in MlaStringIndexOfBenchmark");
    }
}


void String_to_C_LayoutBenchmark() {
    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.");
    mla_c_string_t cStr = mla_string_to_cString(str, false);

    if (cStr.isOwner) {
        delete cStr.c_str;
    }

    mla_c_string_t cStr2 = mla_string_to_cString(str, false);

    if (cStr2.isOwner) {
        delete cStr2.c_str;
    }
}

void String_to_Buffer_LayoutBenchmark() {
    mla_string_t str = mla_string("Hello, World! This is a test string for benchmarking.", 53);
    mla_c_string_t cStr = mla_string_to_cString(str, false);

    if (cStr.isOwner) {
        delete cStr.c_str;
    }

    mla_c_string_t cStr2 = mla_string_to_cString(str, false);

    if (cStr2.isOwner) {
        delete cStr2.c_str;
    }
}

void RefCountBenchmark() {
    mla_char_t *data = mla_create_char_array(1);

    mla_string_t str1 = mla_string_from_buffer_with_ownership(data, 1);
    mla_string_t str2 = str1;
    mla_string_t str3 = str2;
    mla_string_t str4 = str3;
    mla_string_t str5 = str4;
    mla_string_t str6 = str5;
    mla_string_t str7 = str6;
    mla_string_t str8 = str7;
    mla_string_t str9 = str8;
    mla_string_t str10 = str9;
    mla_string_t str11 = str10;
    mla_string_t str12 = str11;
    mla_string_t str13 = str12;
    mla_string_t str14 = str13;
    mla_string_t str15 = str14;
    mla_string_t str16 = str15;
    mla_string_t str17 = str16;
    mla_string_t str18 = str17;
    mla_string_t str19 = str18;
    mla_string_t str20 = str19;
    mla_string_t str21 = str20;
    mla_string_t str22 = str21;
    mla_string_t str23 = str22;
    mla_string_t str24 = str23;
    mla_string_t str25 = str24;
    mla_string_t str26 = str25;
    mla_string_t str27 = str26;
    mla_string_t str28 = str27;
    mla_string_t str29 = str28;
    mla_string_t str30 = str29;
    str1 = mla_string_empty();
    str2 = mla_string_empty();
    str3 = mla_string_empty();
    str4 = mla_string_empty();
    str5 = mla_string_empty();
    str6 = mla_string_empty();
    str7 = mla_string_empty();
    str8 = mla_string_empty();
    str9 = mla_string_empty();
    str10 = mla_string_empty();
    str11 = mla_string_empty();
    str12 = mla_string_empty();
    str13 = mla_string_empty();
    str14 = mla_string_empty();
    str15 = mla_string_empty();
    str16 = mla_string_empty();
    str17 = mla_string_empty();
    str18 = mla_string_empty();
    str19 = mla_string_empty();
    str20 = mla_string_empty();
    str21 = mla_string_empty();
    str22 = mla_string_empty();
    str23 = mla_string_empty();
    str24 = mla_string_empty();
    str25 = mla_string_empty();
    str26 = mla_string_empty();
    str27 = mla_string_empty();
    str28 = mla_string_empty();
    str29 = mla_string_empty();
    str30 = mla_string_empty();
}


void RegisterStringBenchmarks(mla_benchmark_executor_t &p_BenchmarkExecutor) {
    // Concat Benchmarks
    ////////////////////////////////////////////
    mla_benchmark_t benchmark = mla_benchmark("Concat", benchmark_category, StringConcatBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // From Benchmarks
////////////////////////////////////////////
benchmark = mla_benchmark("FromInt8", benchmark_category, StringFromInt8Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromInt16", benchmark_category, StringFromInt16Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromInt32", benchmark_category, StringFromInt32Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromInt64", benchmark_category, StringFromInt64Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUInt8", benchmark_category, StringFromUInt8Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUInt16", benchmark_category, StringFromUInt16Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUInt32", benchmark_category, StringFromUInt32Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUInt64", benchmark_category, StringFromUInt64Benchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromFloat", benchmark_category, StringFromFloatBenchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromDouble", benchmark_category, StringFromDoubleBenchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromBool", benchmark_category, StringFromBoolBenchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUtf16Buffer", benchmark_category, StringFromUtf16BufferBenchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

benchmark = mla_benchmark("FromUtf32Buffer", benchmark_category, StringFromUtf32BufferBenchmark);
mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Ref Count Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("RefCount", benchmark_category, RefCountBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // Contains Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("Contains_Buffer", benchmark_category, StringContains_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("Contains_CStr", benchmark_category, StringContains_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // IndexOf Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("IndexOf_Buffer", benchmark_category, StringIndexOf_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("IndexOf_CStr", benchmark_category, StringIndexOf_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);

    // To C String Benchmarks
    ////////////////////////////////////////////
    benchmark = mla_benchmark("C_String_From_C_Layout", benchmark_category, String_to_C_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
    benchmark = mla_benchmark("C_String_From_Buffer_Layout", benchmark_category, String_to_Buffer_LayoutBenchmark);
    mla_benchmark_executor_register(p_BenchmarkExecutor, benchmark);
}


#endif

//
// Created by chris on 10/15/2025.
//

#ifndef COREOS_MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H
#define COREOS_MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H

#include "windows.h"
#include <tchar.h>

#include "../../core-os/filesystem/mla_file_system.h"
#include "../../core-os/mla_data_types.h"
#include "../../core-os/system/mla_string_concat.h"

struct mla_file_system_native_t {
    mla_string_t basePath;
};

mla_bool_t __mla_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    DWORD attribs = GetFileAttributes((LPCSTR)wideBuffer.data);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = DeleteFile((LPCSTR)wideBuffer.data) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);


    return result;
}

mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = CreateDirectory((LPCSTR)wideBuffer.data, NULL) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    DWORD attribs = GetFileAttributes((LPCSTR)wideBuffer.data);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY);
    mla_string_utf16_buffer_destroy(wideBuffer);


    return result;
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = RemoveDirectory((LPCSTR)wideBuffer.data) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    // Append wildcard for FindFirstFile
    mla_string_t searchPath = mla_string_concat(fullPath, mla_string_const("/*"));

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(searchPath);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((LPCWSTR)wideBuffer.data, &findData);

    mla_string_utf16_buffer_destroy(wideBuffer);
    mla_string_destroy(searchPath);
    mla_string_destroy(fullPath);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        // Skip directories and special entries (. and ..)
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            mla_string_utf16_buffer_t entryBuffer = {(mla_utf_16_char_t*)findData.cFileName, (mla_size_t)wcslen(findData.cFileName)};
            mla_string_t entryName = mla_string_from_utf16_buffer(entryBuffer);
            mla_array_list_add(out_entries, entryName);
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

mla_bool_t __mla_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    // Append wildcard for FindFirstFile
    mla_string_t searchPath = mla_string_concat(fullPath, mla_string_const("/*"));

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(searchPath);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((LPCWSTR)wideBuffer.data, &findData);

    mla_string_utf16_buffer_destroy(wideBuffer);
    mla_string_destroy(searchPath);
    mla_string_destroy(fullPath);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        // Skip special entries (. and ..)
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }

        // Only include directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            mla_string_utf16_buffer_t entryBuffer = {(mla_utf_16_char_t*)findData.cFileName, (mla_size_t)wcslen(findData.cFileName)};
            mla_string_t entryName = mla_string_from_utf16_buffer(entryBuffer);
            mla_array_list_add(out_entries, entryName);
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

mla_buffer_cleanup_mode __mla_file_system_native_close_file(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData;
    HANDLE hFile = reinterpret_cast<HANDLE>(data);

    CloseHandle(hFile);

    return CLEAN_UP_SKIP;
}


mla_bool_t __mla_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {

}

mla_size_t __mla_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {

}

mla_size_t __mla_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {

}

mla_bool_t __mla_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {

}

mla_size_t __mla_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

}

mla_size_t __mla_file_system_native_open_file_write(const mla_stream_output_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    // Determine access mode and creation disposition
    DWORD desiredAccess = 0;
    DWORD creationDisposition = 0;
    mla_bool_t canRead = false;
    mla_bool_t canWrite = false;

    switch (mode) {
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ:
            desiredAccess = GENERIC_READ;
            creationDisposition = OPEN_EXISTING;
            canRead = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE:
            desiredAccess = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            canWrite = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE:
            desiredAccess = GENERIC_READ | GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            canRead = true;
            canWrite = true;
            break;
        default:
            mla_string_utf16_buffer_destroy(wideBuffer);
            mla_string_destroy(fullPath);
            return false;
    }

    HANDLE hFile = CreateFileW(
        (LPCWSTR)wideBuffer.data,
        desiredAccess,
        FILE_SHARE_READ,
        nullptr,
        creationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    mla_string_utf16_buffer_destroy(wideBuffer);

    if (hFile == INVALID_HANDLE_VALUE) {
        mla_string_destroy(fullPath);
        return false;
    }

    if (canRead && canWrite) {
        out_stream = {
            fullPath,
            __mla_file_system_native_open_file_seek, // seek
            __mla_file_system_native_open_file_position, // position
            __mla_file_system_native_open_file_length, // length
            __mla_file_system_native_open_file_set_length, // set_length
            __mla_file_system_native_open_file_read, // read
            __mla_file_system_native_open_file_write, // write
            reinterpret_cast<mla_callback_userdata>(hFile),
            mla_buffer_reference(hFile, true, __mla_file_system_native_close_file)
        };
    } else if (canRead) {
        out_stream = {
            fullPath,
            __mla_file_system_native_open_file_seek, // seek
            __mla_file_system_native_open_file_position, // position
            __mla_file_system_native_open_file_length, // length
            nullptr, // no set_length
            __mla_file_system_native_open_file_read, // read
            nullptr, // no write
            reinterpret_cast<mla_callback_userdata>(hFile),
            mla_buffer_reference(hFile, true, __mla_file_system_native_close_file)
        };
    } else if (canWrite) {
        out_stream = {
            fullPath,
            __mla_file_system_native_open_file_seek, // seek
            __mla_file_system_native_open_file_position, // position
            __mla_file_system_native_open_file_length, // length
            __mla_file_system_native_open_file_set_length, // set_length
            nullptr, // no read
            __mla_file_system_native_open_file_write, // write
            reinterpret_cast<mla_callback_userdata>(hFile),
            mla_buffer_reference(hFile, true, __mla_file_system_native_close_file)
        };
    } else {
        CloseHandle(hFile);
        mla_string_destroy(fullPath);
        return false;
    }

    return true;
}



mla_file_system_t mla_file_system_native_create(mla_string_t basePath) {

    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t *>(mla_malloc(sizeof(mla_file_system_native_t)));

    if (fs == nullptr)
        return mla_file_system_empty();

    mla_memset(fs, 0, sizeof(mla_file_system_native_t));
    fs->basePath = basePath;

    return {
        __mla_file_system_native_file_exists,
        __mla_file_system_native_open_file,
        __mla_file_system_native_delete_file,
        __mla_file_system_native_list_files,
        __mla_file_system_native_create_directory,
        __mla_file_system_native_directory_exists,
        __mla_file_system_native_delete_directory,
        __mla_file_system_native_list_directory,
        reinterpret_cast<mla_callback_userdata>(fs),
        mla_buffer_reference(fs)
    };
};


#endif
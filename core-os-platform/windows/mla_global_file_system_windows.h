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

static const mla_string_t mla_windows_fs_directory_seperator = mla_string_const("\\");

mla_string_t __mla_file_system_native_file_path_to_full_path(mla_file_system_native_t* fs, const mla_string_t& path) {
    // Repleace all slash with backslash
    mla_string_t correctedPath = mla_string_replace(path, mla_fs_directory_seperator, mla_windows_fs_directory_seperator);

    if (fs->basePath.length == 0)
        return correctedPath;

    mla_string_t p1 = fs->basePath;
    mla_string_t p2 = correctedPath;

    if (mla_string_ends_with(p1, mla_windows_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, p1.length - 1);
    }

    if (mla_string_starts_with(p2, mla_windows_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }

    return mla_string_concat(p1, mla_windows_fs_directory_seperator, p2);
}

mla_bool_t __mla_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    DWORD attribs = GetFileAttributesW((LPCWSTR)wideBuffer.data);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY);
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = DeleteFileW((LPCWSTR)wideBuffer.data) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);


    return result;
}

mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = CreateDirectoryW((LPCWSTR)wideBuffer.data, nullptr) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    DWORD attribs = GetFileAttributesW((LPCWSTR)wideBuffer.data);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY);
    mla_string_utf16_buffer_destroy(wideBuffer);


    return result;
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);
    mla_bool_t result = RemoveDirectoryW((LPCWSTR)wideBuffer.data) != 0;
    mla_string_utf16_buffer_destroy(wideBuffer);

    return result;
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

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
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

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
    HANDLE hFile = reinterpret_cast<HANDLE>(stream.user_data);

    LARGE_INTEGER li;
    li.QuadPart = offset;

    return SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN) != 0;
}

mla_size_t __mla_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {
    HANDLE hFile = reinterpret_cast<HANDLE>(stream.user_data);

    LARGE_INTEGER li;
    li.QuadPart = 0;
    LARGE_INTEGER pos;

    if (SetFilePointerEx(hFile, li, &pos, FILE_CURRENT)) {
        return static_cast<mla_size_t>(pos.QuadPart);
    }

    return 0;
}

mla_size_t __mla_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {
    HANDLE hFile = reinterpret_cast<HANDLE>(stream.user_data);

    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(hFile, &fileSize)) {
        return static_cast<mla_size_t>(fileSize.QuadPart);
    }

    return 0;
}

mla_bool_t __mla_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {
    HANDLE hFile = reinterpret_cast<HANDLE>(stream.user_data);

    LARGE_INTEGER li;
    li.QuadPart = length;

    if (SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN)) {
        return SetEndOfFile(hFile) != 0;
    }

    return false;
}

mla_size_t __mla_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    HANDLE hFile = reinterpret_cast<HANDLE>(input.user_data);


    DWORD bytesRead = 0;
    if (ReadFile(hFile, buffer + offset, static_cast<DWORD>(length), &bytesRead, nullptr)) {
        return static_cast<mla_size_t>(bytesRead);
    }

    return 0;
}

mla_size_t __mla_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    HANDLE hFile = reinterpret_cast<HANDLE>(output.user_data);


    DWORD bytesWritten = 0;
    if (WriteFile(hFile, buffer + offset, static_cast<DWORD>(length), &bytesWritten, nullptr)) {
        return static_cast<mla_size_t>(bytesWritten);
    }

    return 0;
}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t*>(file_system.user_data);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

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
            path,
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
            path,
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

mla_buffer_cleanup_mode __mla_file_system_native_cleanup(mla_pointer_t data, mla_callback_userdata userData) {

    (void)userData;

    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t *>(data);

    if (fs == nullptr)
        return CLEAN_UP_SKIP;

    mla_string_destroy(fs->basePath);
    return CLEAN_UP_NEEDED;
}

mla_file_system_t __mla_file_system_native_create_with_base(const mla_string_t& basePath) {

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
        mla_buffer_reference(fs, true, __mla_file_system_native_cleanup)
    };
}


mla_file_system_t mla_file_system_native_create_data_restricted(mla_string_t basePath) {
    // Get the executable path
    WCHAR moduleFileName[32767];
    GetModuleFileNameW(nullptr, moduleFileName, 32767);

    // Find the last backslash (directory separator)
    WCHAR* lastBackslash = wcsrchr(moduleFileName, TEXT('\\'));
    if (lastBackslash != nullptr) {
        // Truncate at the last backslash to get directory path
        *(lastBackslash + 1) = TEXT('\0');
    }

    // Create UTF-16 buffer from TCHAR path
    mla_string_utf16_buffer_t moduleBuffer = {
        (mla_utf_16_char_t*)moduleFileName,
        (mla_size_t)wcslen(moduleFileName)
    };

    // Convert to mla_string_t
    mla_string_t modulePath = mla_string_from_utf16_buffer(moduleBuffer);

    // Append "data\" to the module path
    mla_string_t dataPath = mla_string_concat(modulePath, mla_string_const("data\\"));
    mla_string_destroy(modulePath);

    // Create the data directory if it doesn't exist
    mla_string_utf16_buffer_t dataWideBuffer = mla_string_to_utf16_buffer(dataPath);
    BOOL dataCreated = CreateDirectoryW((LPCWSTR)dataWideBuffer.data, nullptr);
    DWORD dataError = GetLastError();
    mla_string_utf16_buffer_destroy(dataWideBuffer);

    // Check if creation failed (and not because it already exists)
    if (!dataCreated && dataError != ERROR_ALREADY_EXISTS) {
        mla_string_destroy(dataPath);
        return mla_file_system_empty();
    }

    // Append the basePath
    mla_string_t fullPath = mla_string_concat(dataPath, basePath);
    mla_string_destroy(dataPath);

    // Create the final directory if it doesn't exist
    mla_string_utf16_buffer_t fullWideBuffer = mla_string_to_utf16_buffer(fullPath);
    BOOL fullCreated = CreateDirectoryW((LPCWSTR)fullWideBuffer.data, nullptr);
    DWORD fullError = GetLastError();
    mla_string_utf16_buffer_destroy(fullWideBuffer);

    // Check if creation failed (and not because it already exists)
    if (!fullCreated && fullError != ERROR_ALREADY_EXISTS) {
        mla_string_destroy(fullPath);
        return mla_file_system_empty();
    }

    return __mla_file_system_native_create_with_base(fullPath);
}



mla_file_system_t mla_file_system_native_create_global() {

    return __mla_file_system_native_create_with_base(mla_string_empty());

}


#endif
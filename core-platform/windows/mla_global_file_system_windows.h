//
// Created by chris on 10/15/2025.
//

#ifndef MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H
#define MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H

#include "windows.h"
#include <tchar.h>

#include "../../core/filesystem/mla_file_system.h"
#include "../../core/mla_data_types.h"
#include "../../core/system/mla_string_concat.h"

// Maximum path length for Windows file operations (supports UNC paths)
// UNICODE_STRING_MAX_CHARS (32767) + 1 for null terminator
#define MLA_WINDOWS_MAX_PATH (UNICODE_STRING_MAX_CHARS + 1)

struct mla_file_system_native_t {
    mla_string_t basePath;

    static mla_file_system_native_t init() {
        return {
            mla_string_empty()
        };
    }
};

struct mla_file_system_windows_native_stream_t {
    HANDLE syncHandle;
    HANDLE overlappedReadHandle;
};

static const mla_string_t mla_windows_fs_directory_seperator = mla_string_const("\\");

mla_file_system_native_t* __mla_file_system_native_get_native_data(mla_file_system_t& file_system) {

    return mla_pointer_get_data<mla_file_system_native_t>(file_system.resource);
}

mla_string_t __mla_file_system_native_file_path_to_full_path(mla_file_system_native_t* fs, const mla_string_t& path) {
    // Repleace all slash with backslash
    mla_string_t correctedPath = mla_string_replace(path, mla_fs_directory_seperator, mla_windows_fs_directory_seperator);

    if (mla_string_length(fs->basePath) == 0)
        return correctedPath;

    mla_string_t p1 = fs->basePath;
    mla_string_t p2 = correctedPath;

    if (mla_string_ends_with(p1, mla_windows_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, mla_string_length(p1) - 1);
    }

    if (mla_string_starts_with(p2, mla_windows_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }

    return mla_string_concat(p1, mla_windows_fs_directory_seperator, p2);
}

mla_bool_t __mla_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    DWORD attribs = GetFileAttributesW(wide_path);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY);

    return result;
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr) {
        return false;
    }

    mla_bool_t result = DeleteFileW(wide_path) != 0;

    return result;
}


mla_bool_t __mla_file_system_native_create_directory_recursive_wide(LPCWSTR wide_path) {

    // Try to create the directory first
    if (CreateDirectoryW(wide_path, nullptr) != 0) {
        return true;
    }

    DWORD lastError = GetLastError();

    // If directory already exists, that's success
    if (lastError == ERROR_ALREADY_EXISTS) {
        DWORD attribs = GetFileAttributesW(wide_path);
        return attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    // If parent doesn't exist, we need to create parents recursively
    if (lastError == ERROR_PATH_NOT_FOUND) {
        // Find the last backslash to get parent directory
        const WCHAR* lastBackslash = wcsrchr(wide_path, L'\\');

        if (lastBackslash != nullptr && lastBackslash != wide_path) {
            // Calculate parent path length
            mla_size_t parentLen = static_cast<mla_size_t>(lastBackslash - wide_path);

            // Use stack buffer for parent path (Windows supports up to 32767 character paths)
            WCHAR parentPath[MLA_WINDOWS_MAX_PATH];

            // Copy parent path
            if (parentLen >= MLA_WINDOWS_MAX_PATH) {
                return false;
            }

            wcsncpy_s(parentPath, MLA_WINDOWS_MAX_PATH, wide_path, parentLen);
            parentPath[parentLen] = L'\0';

            // Recursively create parent directory
            if (!__mla_file_system_native_create_directory_recursive_wide(parentPath)) {
                return false;
            }

            // Now try to create the target directory
            if (CreateDirectoryW(wide_path, nullptr) != 0) {
                return true;
            }

            lastError = GetLastError();
            if (lastError == ERROR_ALREADY_EXISTS) {
                DWORD attribs = GetFileAttributesW(wide_path);
                return attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY);
            }
        }
    }

    return false;
}


mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    mla_bool_t result = __mla_file_system_native_create_directory_recursive_wide(wide_path);
    mla_string_destroy(fullPath);

    return result;
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    DWORD attribs = GetFileAttributesW(wide_path);
    mla_bool_t result = attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY);

    return result;
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    mla_bool_t result = RemoveDirectoryW(wide_path) != 0;

    return result;
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    // Append wildcard for FindFirstFile
    mla_string_t searchPath = mla_string_concat(fullPath, mla_string_const("/*"));

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(searchPath);

    WIN32_FIND_DATAW findData;

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    HANDLE hFind = FindFirstFileW(wide_path, &findData);

    mla_string_destroy(searchPath);
    mla_string_destroy(fullPath);

    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        // Skip directories and special entries (. and ..)
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            mla_string_utf16_buffer_t entryBuffer = {mla_platform_pointer_to_managed_pointer(findData.cFileName), (mla_size_t)wcslen(findData.cFileName)};
            mla_string_t entryName = mla_string_from_utf16_buffer(entryBuffer);
            mla_array_list_add(out_entries, entryName);
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

mla_bool_t __mla_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    // Append wildcard for FindFirstFile
    mla_string_t searchPath = mla_string_concat(fullPath, mla_string_const("/*"));

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(searchPath);

    WIN32_FIND_DATAW findData;

    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;

    HANDLE hFind = FindFirstFileW(wide_path, &findData);

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
            mla_string_utf16_buffer_t entryBuffer = {mla_platform_pointer_to_managed_pointer(findData.cFileName), (mla_size_t)wcslen(findData.cFileName)};
            mla_string_t entryName = mla_string_from_utf16_buffer(entryBuffer);
            mla_array_list_add(out_entries, entryName);
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
    return true;
}

mla_file_system_windows_native_stream_t* __mla_file_system_native_get_stream_data(const mla_file_system_stream_t& stream) {

    mla_native_resource_t* native_resource = mla_native_resource_from_managed_pointer(stream.resource);

    if (native_resource == nullptr || native_resource->asPointer == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<mla_file_system_windows_native_stream_t*>(native_resource->asPointer);
}

void __mla_file_system_native_close_file(const mla_native_resource_t& userData) {

    mla_file_system_windows_native_stream_t* streamData = reinterpret_cast<mla_file_system_windows_native_stream_t*>(userData.asPointer);

    if (streamData == nullptr) {
        return;
    }

    if (streamData->overlappedReadHandle != INVALID_HANDLE_VALUE && streamData->overlappedReadHandle != nullptr) {
        CloseHandle(streamData->overlappedReadHandle);
        streamData->overlappedReadHandle = INVALID_HANDLE_VALUE;
    }

    if (streamData->syncHandle != INVALID_HANDLE_VALUE && streamData->syncHandle != nullptr) {
        CloseHandle(streamData->syncHandle);
        streamData->syncHandle = INVALID_HANDLE_VALUE;
    }

    mla_platform_free(streamData);
}

HANDLE __mla_file_system_native_get_handle_from_stream(const mla_file_system_stream_t& stream) {

    mla_file_system_windows_native_stream_t* streamData = __mla_file_system_native_get_stream_data(stream);

    if (streamData == nullptr) {
        return INVALID_HANDLE_VALUE;
    }

    return streamData->syncHandle;

}

HANDLE __mla_file_system_native_get_overlapped_handle_from_stream(const mla_file_system_stream_t& stream) {

    mla_file_system_windows_native_stream_t* streamData = __mla_file_system_native_get_stream_data(stream);

    if (streamData == nullptr) {
        return INVALID_HANDLE_VALUE;
    }

    return streamData->overlappedReadHandle;
}

mla_bool_t __mla_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(stream);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    LARGE_INTEGER li;
    li.QuadPart = offset;

    return SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN) != 0;
}

mla_size_t __mla_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(stream);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }


    LARGE_INTEGER li;
    li.QuadPart = 0;
    LARGE_INTEGER pos;

    if (SetFilePointerEx(hFile, li, &pos, FILE_CURRENT)) {
        return static_cast<mla_size_t>(pos.QuadPart);
    }

    return 0;
}

mla_size_t __mla_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(stream);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }


    LARGE_INTEGER fileSize;
    if (GetFileSizeEx(hFile, &fileSize)) {
        return static_cast<mla_size_t>(fileSize.QuadPart);
    }

    return 0;
}

mla_bool_t __mla_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(stream);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }


    LARGE_INTEGER li;
    li.QuadPart = length;

    if (SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN)) {
        return SetEndOfFile(hFile) != 0;
    }

    return false;
}

mla_size_t __mla_file_system_native_open_file_read_sync(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(input);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    DWORD bytesRead = 0;
    if (ReadFile(hFile, buffer + offset, static_cast<DWORD>(length), &bytesRead, nullptr)) {
        return static_cast<mla_size_t>(bytesRead);
    }

    return 0;
}

mla_size_t __mla_file_system_native_open_file_read_count(mla_size_t length) {

    const mla_size_t targetChunkSize = 512u * 1024u;
    const mla_size_t maxOutstandingReads = 16u;

    if (length <= targetChunkSize) {
        return 1;
    }

    mla_size_t readCount = (length + targetChunkSize - 1u) / targetChunkSize;
    if (readCount > maxOutstandingReads) {
        readCount = maxOutstandingReads;
    }

    return readCount;
}

mla_size_t __mla_file_system_native_open_file_read_parallel(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    const mla_size_t maxOutstandingReads = 16u;

    HANDLE syncHandle = __mla_file_system_native_get_handle_from_stream(input);
    HANDLE overlappedHandle = __mla_file_system_native_get_overlapped_handle_from_stream(input);

    if (syncHandle == INVALID_HANDLE_VALUE || overlappedHandle == INVALID_HANDLE_VALUE) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    mla_size_t readCount = __mla_file_system_native_open_file_read_count(length);
    if (readCount <= 1u) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    LARGE_INTEGER currentPosition;
    LARGE_INTEGER zeroOffset;
    zeroOffset.QuadPart = 0;

    if (!SetFilePointerEx(syncHandle, zeroOffset, &currentPosition, FILE_CURRENT)) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    OVERLAPPED overlapped[maxOutstandingReads] = {};
    HANDLE events[maxOutstandingReads] = {};

    mla_size_t totalBytesRead = 0;
    mla_size_t bytesScheduled = 0;
    const mla_size_t baseChunkSize = length / readCount;
    const mla_size_t remainder = length % readCount;

    for (mla_size_t i = 0; i < readCount; ++i) {
        const mla_size_t chunkSize = baseChunkSize + (i < remainder ? 1u : 0u);
        if (chunkSize == 0u) {
            continue;
        }

        events[i] = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (events[i] == nullptr) {
            for (mla_size_t j = 0; j < i; ++j) {
                if (events[j] != nullptr) {
                    CancelIoEx(overlappedHandle, &overlapped[j]);
                    CloseHandle(events[j]);
                }
            }
            return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
        }

        overlapped[i].hEvent = events[i];

        const mla_uint64_t fileOffset = static_cast<mla_uint64_t>(currentPosition.QuadPart) + bytesScheduled;
        overlapped[i].Offset = static_cast<DWORD>(fileOffset & 0xFFFFFFFFu);
        overlapped[i].OffsetHigh = static_cast<DWORD>((fileOffset >> 32u) & 0xFFFFFFFFu);

        BOOL readResult = ReadFile(
            overlappedHandle,
            buffer + offset + bytesScheduled,
            static_cast<DWORD>(chunkSize),
            nullptr,
            &overlapped[i]
        );

        if (!readResult && GetLastError() != ERROR_IO_PENDING) {
            CloseHandle(events[i]);
            events[i] = nullptr;

            for (mla_size_t j = 0; j < i; ++j) {
                if (events[j] != nullptr) {
                    CancelIoEx(overlappedHandle, &overlapped[j]);
                    WaitForSingleObject(events[j], INFINITE);
                    CloseHandle(events[j]);
                    events[j] = nullptr;
                }
            }

            return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
        }

        bytesScheduled += chunkSize;
    }

    if (readCount > 0u) {
        DWORD waitResult = WaitForMultipleObjects(static_cast<DWORD>(readCount), events, TRUE, INFINITE);
        if (waitResult < WAIT_OBJECT_0 || waitResult >= (WAIT_OBJECT_0 + readCount)) {
            for (mla_size_t i = 0; i < readCount; ++i) {
                if (events[i] != nullptr) {
                    CancelIoEx(overlappedHandle, &overlapped[i]);
                    CloseHandle(events[i]);
                }
            }
            return 0;
        }
    }

    for (mla_size_t i = 0; i < readCount; ++i) {
        if (events[i] == nullptr) {
            continue;
        }

        DWORD bytesRead = 0;
        if (GetOverlappedResult(overlappedHandle, &overlapped[i], &bytesRead, FALSE)) {
            totalBytesRead += static_cast<mla_size_t>(bytesRead);
        }

        CloseHandle(events[i]);
    }

    LARGE_INTEGER newPosition;
    newPosition.QuadPart = currentPosition.QuadPart + totalBytesRead;
    SetFilePointerEx(syncHandle, newPosition, nullptr, FILE_BEGIN);

    return totalBytesRead;
}

mla_size_t __mla_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    return __mla_file_system_native_open_file_read_parallel(input, offset, length, buffer);
}

mla_size_t __mla_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    HANDLE hFile = __mla_file_system_native_get_handle_from_stream(output);

    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;
    }


    DWORD bytesWritten = 0;
    if (WriteFile(hFile, buffer + offset, static_cast<DWORD>(length), &bytesWritten, nullptr)) {
        return static_cast<mla_size_t>(bytesWritten);
    }

    return 0;
}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    mla_string_utf16_buffer_t wideBuffer = mla_string_to_utf16_buffer(fullPath);


    LPCWSTR wide_path = mla_pointer_get_data<WCHAR>(wideBuffer.data);

    if (wide_path == nullptr)
        return false;


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
            mla_string_destroy(fullPath);
            return false;
    }

    HANDLE hFile = CreateFileW(
        wide_path,
        desiredAccess,
        FILE_SHARE_READ,
        nullptr,
        creationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        mla_string_destroy(fullPath);
        return false;
    }

    HANDLE overlappedReadHandle = INVALID_HANDLE_VALUE;
    if (canRead) {
        overlappedReadHandle = CreateFileW(
            wide_path,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            nullptr
        );
    }

    mla_file_system_windows_native_stream_t* streamData = static_cast<mla_file_system_windows_native_stream_t*>(mla_platform_malloc(sizeof(mla_file_system_windows_native_stream_t)));
    if (streamData == nullptr) {
        if (overlappedReadHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(overlappedReadHandle);
        }
        CloseHandle(hFile);
        mla_string_destroy(fullPath);
        return false;
    }

    streamData->syncHandle = hFile;
    streamData->overlappedReadHandle = overlappedReadHandle;

    mla_native_resource_t resource = mla_dynamic_data_from_pointer(streamData);
    mla_pointer_t resource_ptr = mla_native_resource_to_managed_pointer(resource, __mla_file_system_native_close_file);

    if (canRead && canWrite) {
        out_stream = {
            path,
            __mla_file_system_native_open_file_seek, // seek
            __mla_file_system_native_open_file_position, // position
            __mla_file_system_native_open_file_length, // length
            __mla_file_system_native_open_file_set_length, // set_length
            __mla_file_system_native_open_file_read, // read
            __mla_file_system_native_open_file_write, // write
            resource_ptr
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
            resource_ptr
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
            resource_ptr
        };
    } else {
        if (overlappedReadHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(overlappedReadHandle);
        }
        CloseHandle(hFile);
        mla_platform_free(streamData);
        mla_string_destroy(fullPath);
        return false;
    }

    return true;
}

mla_file_system_t __mla_file_system_native_create_with_base(const mla_string_t& basePath) {

    mla_pointer_t fs_ptr = mla_malloc_struct(mla_file_system_native_t);

    mla_file_system_native_t* fs = mla_pointer_get_data<mla_file_system_native_t>(fs_ptr);

    if (fs == nullptr)
        return mla_file_system_empty();

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
        fs_ptr
    };
}

mla_file_system_t mla_file_system_native_create_restricted(mla_string_t full_basePath) {
    return __mla_file_system_native_create_with_base(full_basePath);
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
        mla_platform_pointer_to_managed_pointer(moduleFileName),
        (mla_size_t)wcslen(moduleFileName)
    };

    // Convert to mla_string_t
    mla_string_t modulePath = mla_string_from_utf16_buffer(moduleBuffer);

    // Append "data\" to the module path
    mla_string_t dataPath = mla_string_concat(modulePath, mla_string_const("data\\"));
    mla_string_destroy(modulePath);

    // Create the data directory if it doesn't exist
    mla_string_utf16_buffer_t dataWideBuffer = mla_string_to_utf16_buffer(dataPath);

    LPCWSTR dataWideBuffer_ptr = mla_pointer_get_data<WCHAR>(dataWideBuffer.data);

    if (dataWideBuffer_ptr == nullptr)
        return mla_file_system_empty();

    BOOL dataCreated = CreateDirectoryW(dataWideBuffer_ptr, nullptr);
    DWORD dataError = GetLastError();

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

    LPCWSTR fullWideBuffer_ptr = mla_pointer_get_data<WCHAR>(dataWideBuffer.data);

    if (fullWideBuffer_ptr == nullptr)
        return mla_file_system_empty();

    BOOL fullCreated = CreateDirectoryW(fullWideBuffer_ptr, nullptr);
    DWORD fullError = GetLastError();

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
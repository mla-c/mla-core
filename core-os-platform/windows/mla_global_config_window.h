//
// Created by chris on 10/1/2025.
//

#ifndef COREOS_MLA_GLOBAL_CONFIG_WINDOW_H
#define COREOS_MLA_GLOBAL_CONFIG_WINDOW_H

#include "../../core-os/config/mla_config.h"
#include "windows.h"
#include <tchar.h>

#if !defined(mla_windows_max_config_file_size)
#define mla_windows_max_config_file_size (128 * 1024) // 128KB is default
#endif

#define mla_commit_buffer_size 32767
void get_config_filename(TCHAR* buffer, size_t bufferSize) {

    TCHAR exePath[mla_commit_buffer_size];
    GetModuleFileName(nullptr, exePath, mla_commit_buffer_size);

    // Get just the filename part
    TCHAR* lastBackslash = _tcsrchr(exePath, TEXT('\\'));
    TCHAR* filename = lastBackslash ? lastBackslash + 1 : exePath;

    // Copy the filename to the buffer
    _tcscpy_s(buffer, bufferSize, filename);

    // Add .config extension
    _tcscat_s(buffer, bufferSize, TEXT(".config"));

}


// On Windows the configuration is just an file in the application directory
// The file can be read and written using standard file operations

mla_bytes_t __windows_read_config_input() {

    TCHAR configFilename[mla_commit_buffer_size];
    get_config_filename(configFilename, mla_commit_buffer_size);

    // Open the config file
    HANDLE hFile = CreateFile(
        configFilename,     // File name
        GENERIC_READ,                     // Open for reading
        FILE_SHARE_READ,                  // Share for reading
        nullptr,                             // Default security
        OPEN_EXISTING,                    // Only open if exists
        FILE_ATTRIBUTE_NORMAL,            // Normal file attribute
        nullptr                              // No template
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Failed to open file
        return mla_bytes_empty();
    }

    // Get the file size
    DWORD fileSize = GetFileSize(hFile, nullptr);

    if (fileSize == INVALID_FILE_SIZE || fileSize > mla_windows_max_config_file_size) {
        CloseHandle(hFile);
        return mla_bytes_empty();
    }

    // Allocate buffer for file content
    mla_bytes_t config_data = mla_bytes(fileSize);

    if (config_data.size != fileSize) {

        CloseHandle(hFile);
        return config_data;
    }

    // Read file content
    DWORD bytesRead;
    BOOL readResult = ReadFile(
        hFile,                 // File handle
        mla_bytes_get_data_for_writing(config_data),      // Buffer
        fileSize,              // Number of bytes to read
        &bytesRead,            // Number of bytes read
        nullptr                   // No overlapped I/O
    );

    // Close the file
    CloseHandle(hFile);

    // Check if read was successful
    if (!readResult || (bytesRead != fileSize)) {
        mla_bytes_destroy(config_data);
        config_data = mla_bytes(0);
    }

    return config_data;


}

mla_bytes_t __windows_create_config_output_buffer() {

    return mla_bytes(mla_windows_max_config_file_size);
}

mla_bool_t __windows_commit_config_output(mla_bytes_t& output) {


    TCHAR exePath[mla_commit_buffer_size];
    if (GetModuleFileName(nullptr, exePath, mla_commit_buffer_size) == 0) {
        mla_bytes_destroy(output);
        return false;
    }

    // Use _tcschr instead of strrchr/wcsrchr
    TCHAR* lastBackslash = _tcsrchr(exePath, TEXT('\\'));
    if (lastBackslash == nullptr) {
        mla_bytes_destroy(output);
        return false;
    }
    *(lastBackslash + 1) = TEXT('\0');

    // Use TCHAR for all paths
    TCHAR configPath[mla_commit_buffer_size];
    TCHAR tempPath[mla_commit_buffer_size];
    TCHAR backupPath[mla_commit_buffer_size];

    // Then modify the line you highlighted to:
    TCHAR configFilename[mla_commit_buffer_size];
    get_config_filename(configFilename, mla_commit_buffer_size);

    _tcscpy_s(configPath, mla_commit_buffer_size, exePath);
    _tcscat_s(configPath, mla_commit_buffer_size, configFilename);

    _tcscpy_s(tempPath, mla_commit_buffer_size, exePath);
    _tcscat_s(tempPath, mla_commit_buffer_size, configFilename);
    _tcscat_s(tempPath, mla_commit_buffer_size, TEXT(".tmp"));

    _tcscpy_s(backupPath, mla_commit_buffer_size, exePath);
    _tcscat_s(backupPath, mla_commit_buffer_size, configFilename);
    _tcscat_s(backupPath, mla_commit_buffer_size, TEXT(".bak"));

    // Delete temporary file if it exists
    DeleteFile(tempPath);

    // Create and open the temporary file for writing
    HANDLE hTempFile = CreateFile(
        tempPath,             // Absolute path to temporary file
        GENERIC_WRITE,        // Open for writing
        0,                    // No sharing
        nullptr,              // Default security
        CREATE_ALWAYS,        // Always create new file
        FILE_ATTRIBUTE_NORMAL,// Normal file attribute
        nullptr               // No template
    );

    if (hTempFile == INVALID_HANDLE_VALUE) {
        mla_bytes_destroy(output);
        return false;
    }

    // Write data to temporary file
    DWORD bytesWritten;
    BOOL writeResult = WriteFile(
        hTempFile,           // File handle
        output.data,         // Buffer with data to write
        output.size,         // Number of bytes to write
        &bytesWritten,       // Number of bytes written
        nullptr              // No overlapped I/O
    );

    // Close the temporary file
    CloseHandle(hTempFile);

    if ((writeResult == FALSE) || bytesWritten != output.size) {
        DeleteFile(tempPath);
        mla_bytes_destroy(output);
        return false;
    }

    // Replace the original file with the temporary file
    BOOL replaceResult = ReplaceFile(
        configPath,          // Target file (absolute path)
        tempPath,            // Source file (absolute path)
        nullptr,             // Backup file name (none)
        0,                   // Replace flags
        nullptr,             // Reserved
        nullptr              // Reserved
    );

    // If ReplaceFile failed, try the delete-and-rename approach
    if ((replaceResult == FALSE)) {
        // Remove any existing backup file
        DeleteFile(backupPath);

        // Check If orginal exists
        if (GetFileAttributes(configPath) != INVALID_FILE_ATTRIBUTES) {

            // First rename the original file to backup
            BOOL backupResult = MoveFileEx(configPath, backupPath, MOVEFILE_REPLACE_EXISTING);

            if (!backupResult) {
                DeleteFile(tempPath);
                mla_bytes_destroy(output);
                return false;
            }
        }


        // Now move the temp file to the original location
        if (!MoveFileEx(tempPath, configPath, MOVEFILE_REPLACE_EXISTING)) {
            MoveFileEx(backupPath, configPath, MOVEFILE_REPLACE_EXISTING);
            DeleteFile(tempPath);
            mla_bytes_destroy(output);
            return false;
        }

        // Success - delete the backup file
        DeleteFile(backupPath);
    }

    mla_bytes_destroy(output);
    return true;
}


mla_config_low_level_operations_t g_low_level_operations = {
    __windows_read_config_input,
    __windows_create_config_output_buffer,
    __windows_commit_config_output
};

#endif
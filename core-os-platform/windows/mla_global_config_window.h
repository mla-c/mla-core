//
// Created by chris on 10/1/2025.
//

#ifndef COREOS_MLA_GLOBAL_CONFIG_WINDOW_H
#define COREOS_MLA_GLOBAL_CONFIG_WINDOW_H

#include "../../core-os/config/mla_config.h"
#include "windows.h"

#if !defined(mla_windows_max_config_file_size)
#define mla_windows_max_config_file_size (128 * 1024) // 128KB is default
#endif

#define mla_windows_config_file_name "app_config.config"

// On Windows the configuration is just an file in the application directory
// The file can be read and written using standard file operations

mla_bytes_t __windows_read_config_input() {


    // Open the config file
    HANDLE hFile = CreateFile(
        mla_windows_config_file_name,     // File name
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

    // Create a new file beside the old one and then rename it
    // This is to avoid corrupting the config file if something goes wrong during writing

    // Create temporary file name (append .tmp to the original filename)
    const mla_char_t* tempFileName = "app_config.config.tmp";

    // If the temporary file already exists, delete it
    DeleteFile(tempFileName);

    // Create and open the temporary file for writing
    HANDLE hTempFile = CreateFile(
        tempFileName,         // Temporary file name
        GENERIC_WRITE,        // Open for writing
        0,                    // No sharing
        nullptr,              // Default security
        CREATE_ALWAYS,        // Always create new file
        FILE_ATTRIBUTE_NORMAL,// Normal file attribute
        nullptr               // No template
    );

    if (hTempFile == INVALID_HANDLE_VALUE) {
        // Failed to create temporary file
        mla_bytes_destroy(output);
        return false;
    }

    // Write data to temporary file
    DWORD bytesWritten;
    BOOL writeResult = WriteFile(
        hTempFile,                           // File handle
        output.data, // Buffer with data to write
        output.size,                         // Number of bytes to write
        &bytesWritten,                       // Number of bytes written
        nullptr                              // No overlapped I/O
    );

    // Close the temporary file
    CloseHandle(hTempFile);

    if (!writeResult || bytesWritten != output.size) {
        // Write failed or was incomplete
        DeleteFile(tempFileName);
        mla_bytes_destroy(output);
        return false;
    }

    // Replace the original file with the temporary file
    BOOL replaceResult = ReplaceFile(
        mla_windows_config_file_name,  // Target file
        tempFileName,                  // Source file
        nullptr,                       // Backup file name (none)
        0,                             // Replace flags
        nullptr,                       // Reserved
        nullptr                        // Reserved
    );

    // If ReplaceFile failed, try the delete-and-rename approach
    if (!replaceResult) {
        // Create backup filename
        const mla_char_t* backupFileName = "app_config.config.bak";

        // Remove any existing backup file
        DeleteFile(backupFileName);

        // First rename the original file to backup
        BOOL backupResult = MoveFile(mla_windows_config_file_name, backupFileName);

        if (!backupResult) {
            // Failed to create backup, abort operation
            DeleteFile(tempFileName);
            mla_bytes_destroy(output);
            return false;
        }

        // Now move the temp file to the original location
        if (!MoveFile(tempFileName, mla_windows_config_file_name)) {
            // Failed to move temp file, restore from backup
            MoveFile(backupFileName, mla_windows_config_file_name);
            DeleteFile(tempFileName);
            mla_bytes_destroy(output);
            return false;
        }

        // Success - delete the backup file
        DeleteFile(backupFileName);
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
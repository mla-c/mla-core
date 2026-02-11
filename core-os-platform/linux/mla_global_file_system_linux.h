//
// Created by christian on 10/17/25.
//

#ifndef COREOS_MLA_GLOBAL_FILE_SYSTEM_LINUX_H
#define COREOS_MLA_GLOBAL_FILE_SYSTEM_LINUX_H

#include "../../core-os/filesystem/mla_file_system.h"
#include "../../core-os/mla_data_types.h"
#include "../../core-os/system/mla_string_concat.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

struct mla_file_system_native_t {
    mla_string_t basePath;
};


static const mla_string_t mla_linux_fs_directory_seperator = mla_string_const("/");

mla_string_t __mla_file_system_native_file_path_to_full_path(mla_file_system_native_t* fs, const mla_string_t& path) {
    // Replace backslashes with forward slashes for Linux
    mla_string_t correctedPath = mla_string_replace(path, mla_string_const("\\"), mla_linux_fs_directory_seperator);

    if (mla_string_length(fs->basePath) == 0)
        return correctedPath;

    mla_string_t p1 = fs->basePath;
    mla_string_t p2 = correctedPath;

    if (mla_string_ends_with(p1, mla_linux_fs_directory_seperator)) {
        p1 = mla_string_substr(p1, 0, mla_string_length(p1) - 1);
    }

    if (mla_string_starts_with(p2, mla_linux_fs_directory_seperator)) {
        p2 = mla_string_substr(p2, 1);
    }

    return mla_string_concat(p1, mla_linux_fs_directory_seperator, p2);
}

mla_bool_t __mla_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISREG(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = unlink(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    return result == 0;
}

mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = mkdir(mla_string_data(fullPath), 0755); // rwxr-xr-x permissions
    mla_string_destroy(fullPath);

    return result == 0 || errno == EEXIST;
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISDIR(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = rmdir(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    return result == 0;
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    if (dir == NULL) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and special entries (. and ..)
        if (entry->d_type != DT_REG) {
            continue;
        }

        mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
        mla_array_list_add(out_entries, entryName);
    }

    closedir(dir);
    return true;
}

mla_bool_t __mla_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    DIR* dir = opendir(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    if (dir == NULL) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip special entries (. and ..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Only include directories
        if (entry->d_type == DT_DIR) {
            mla_string_t entryName = mla_string_copy(entry->d_name, mla_strlen(entry->d_name));
            mla_array_list_add(out_entries, entryName);
        }
    }

    closedir(dir);
    return true;
}

mla_buffer_cleanup_mode __mla_file_system_native_close_file(mla_pointer_t data, const mla_dynamic_data_t& userData) {
    (void)userData;
    int fd = *reinterpret_cast<int*>(data);

    close(fd);
    mla_free(data);

    return CLEAN_UP_SKIP;
}

mla_bool_t __mla_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {
    int* fd = static_cast<int*>(stream.resource.asPointer);

    return lseek(*fd, offset, SEEK_SET) != -1;
}

mla_size_t __mla_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {
    int* fd = static_cast<int*>(stream.resource.asPointer);

    off_t pos = lseek(*fd, 0, SEEK_CUR);
    return pos != -1 ? static_cast<mla_size_t>(pos) : 0;
}

mla_size_t __mla_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {
    int* fd = static_cast<int*>(stream.resource.asPointer);

    // Save current position
    off_t currentPos = lseek(*fd, 0, SEEK_CUR);
    if (currentPos == -1) return 0;

    // Seek to end and get position
    off_t size = lseek(*fd, 0, SEEK_END);

    // Restore original position
    lseek(*fd, currentPos, SEEK_SET);

    return size != -1 ? static_cast<mla_size_t>(size) : 0;
}

mla_bool_t __mla_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {
    int* fd = static_cast<int*>(stream.resource.asPointer);

    return ftruncate(*fd, length) == 0;
}

mla_size_t __mla_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    int* fd = static_cast<int*>(input.resource.asPointer);

    ssize_t bytesRead = read(*fd, buffer + offset, length);
    return bytesRead > 0 ? static_cast<mla_size_t>(bytesRead) : 0;
}

mla_size_t __mla_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    int* fd = static_cast<int*>(output.resource.asPointer);

    ssize_t bytesWritten = write(*fd, buffer + offset, length);
    return bytesWritten > 0 ? static_cast<mla_size_t>(bytesWritten) : 0;
}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {
    mla_file_system_native_t* fs = static_cast<mla_file_system_native_t*>(file_system.user_data.asPointer);
    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int flags = 0;
    mla_bool_t canRead = false;
    mla_bool_t canWrite = false;

    switch (mode) {
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ:
            flags = O_RDONLY;
            canRead = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE:
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            canWrite = true;
            break;
        case MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE:
            flags = O_RDWR | O_CREAT;
            canRead = true;
            canWrite = true;
            break;
        default:
            mla_string_destroy(fullPath);
            return false;
    }

    int fd = open(mla_string_data(fullPath), flags, 0644); // rw-r--r-- permissions

    if (fd == -1) {
        mla_string_destroy(fullPath);
        return false;
    }

    // Allocate memory to store the file descriptor (needs to be freed in cleanup)
    int* fdPtr = static_cast<int*>(mla_malloc(sizeof(int)));
    if (fdPtr == nullptr) {
        close(fd);
        mla_string_destroy(fullPath);
        return false;
    }

    *fdPtr = fd;

    if (canRead && canWrite) {
        out_stream = {
            path,
            __mla_file_system_native_open_file_seek,
            __mla_file_system_native_open_file_position,
            __mla_file_system_native_open_file_length,
            __mla_file_system_native_open_file_set_length,
            __mla_file_system_native_open_file_read,
            __mla_file_system_native_open_file_write,
            mla_dynamic_data_from_pointer(fdPtr),
            mla_buffer_reference_create(fdPtr, true, __mla_file_system_native_close_file, mla_dynamic_data_empty())
        };
    } else if (canRead) {
        out_stream = {
            path,
            __mla_file_system_native_open_file_seek,
            __mla_file_system_native_open_file_position,
            __mla_file_system_native_open_file_length,
            nullptr,
            __mla_file_system_native_open_file_read,
            nullptr,
            mla_dynamic_data_from_pointer(fdPtr),
            mla_buffer_reference_create(fdPtr, true, __mla_file_system_native_close_file, mla_dynamic_data_empty())
        };
    } else if (canWrite) {
        out_stream = {
            path,
            __mla_file_system_native_open_file_seek,
            __mla_file_system_native_open_file_position,
            __mla_file_system_native_open_file_length,
            __mla_file_system_native_open_file_set_length,
            nullptr,
            __mla_file_system_native_open_file_write,
            mla_dynamic_data_from_pointer(fdPtr),
            mla_buffer_reference_create(fdPtr, true, __mla_file_system_native_close_file, mla_dynamic_data_empty())
        };
    } else {
        close(fd);
        mla_free(fdPtr);
        mla_string_destroy(fullPath);
        return false;
    }

    mla_string_destroy(fullPath);
    return true;
}

mla_buffer_cleanup_mode __mla_file_system_native_cleanup(mla_pointer_t data, const mla_dynamic_data_t& userData) {

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
        mla_dynamic_data_from_pointer(fs),
        mla_buffer_reference_create(fs, true, __mla_file_system_native_cleanup, mla_dynamic_data_empty())
    };
}

mla_file_system_t mla_file_system_native_create_data_restricted(mla_string_t basePath) {
    // Get the executable path
    char exePath[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);

    if (len == -1) {
        return mla_file_system_empty();
    }

    exePath[len] = '\0';

    // Find last slash to get directory
    char* lastSlash = strrchr(exePath, '/');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = '\0';
    }

    // Create path string
    mla_string_t modulePath = mla_string(exePath);

    // Append "data/" to the module path
    mla_string_t dataPath = mla_string_concat(modulePath, mla_string_const("data/"));
    mla_string_destroy(modulePath);

    // Create the data directory if it doesn't exist
    mkdir(mla_string_data(dataPath), 0755);

    // Append the basePath
    mla_string_t fullPath = mla_string_concat(dataPath, basePath);
    mla_string_destroy(dataPath);

    // Create the final directory if it doesn't exist
    mkdir(mla_string_data(fullPath), 0755);

    return __mla_file_system_native_create_with_base(fullPath);
}



mla_file_system_t mla_file_system_native_create_global() {

    return __mla_file_system_native_create_with_base(mla_string_empty());

}


#endif
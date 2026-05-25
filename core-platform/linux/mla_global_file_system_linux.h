//
// Created by christian on 10/17/25.
//

#ifndef MLA_GLOBAL_FILE_SYSTEM_LINUX_H
#define MLA_GLOBAL_FILE_SYSTEM_LINUX_H

#include "../../core/filesystem/mla_file_system.h"
#include "../../core/mla_data_types.h"
#include "../../core/mla_native_resource.h"
#include "../../core/system/mla_string_concat.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <cstring>
#include <pthread.h>

struct mla_file_system_native_t {
    mla_string_t basePath;

    static mla_file_system_native_t init() {
        return {
            mla_string_empty()
        };
    }
};


static const mla_string_t mla_linux_fs_directory_seperator = mla_string_const("/");

mla_file_system_native_t* __mla_file_system_native_get_native_data(mla_file_system_t& file_system) {

    return mla_pointer_get_data<mla_file_system_native_t>(file_system.resource);
}

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

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISREG(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = unlink(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    return result == 0;
}

mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = mkdir(mla_string_data(fullPath), 0755); // rwxr-xr-x permissions
    mla_string_destroy(fullPath);

    return result == 0 || errno == EEXIST;
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    struct stat statbuf;
    int result = stat(mla_string_data(fullPath), &statbuf);
    mla_bool_t exists = (result == 0 && S_ISDIR(statbuf.st_mode));

    mla_string_destroy(fullPath);
    return exists;
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

    mla_string_t fullPath = __mla_file_system_native_file_path_to_full_path(fs, path);

    int result = rmdir(mla_string_data(fullPath));
    mla_string_destroy(fullPath);

    return result == 0;
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

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

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

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

void __mla_file_system_native_close_file(const mla_native_resource_t& userData) {

    int* fdPtr = reinterpret_cast<int*>(userData.asPointer);

    if (fdPtr == nullptr)
        return;

    close(*fdPtr);
    mla_platform_free(fdPtr);
}

int __mla_file_system_native_get_fd_from_stream(const mla_file_system_stream_t& stream) {

    mla_native_resource_t* native_resource = mla_native_resource_from_managed_pointer(stream.resource);

    if (native_resource == nullptr)
        return -1;

    return *reinterpret_cast<int*>(native_resource->asPointer);
}

struct mla_file_system_linux_parallel_read_task_t {
    int fd;
    off_t fileOffset;
    mla_byte_t* buffer;
    mla_size_t length;
    mla_size_t bytesRead;
    mla_bool_t failed;
};

void* __mla_file_system_native_open_file_read_worker(void* context) {

    mla_file_system_linux_parallel_read_task_t* task = static_cast<mla_file_system_linux_parallel_read_task_t*>(context);

    if (task == nullptr) {
        return nullptr;
    }

    task->bytesRead = 0;
    task->failed = false;

    while (task->bytesRead < task->length) {
        ssize_t readResult = pread(
            task->fd,
            task->buffer + task->bytesRead,
            task->length - task->bytesRead,
            task->fileOffset + static_cast<off_t>(task->bytesRead)
        );

        if (readResult < 0) {
            task->failed = true;
            return nullptr;
        }

        if (readResult == 0) {
            return nullptr;
        }

        task->bytesRead += static_cast<mla_size_t>(readResult);
    }

    return nullptr;
}

mla_size_t __mla_file_system_native_open_file_read_sync(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    int fd = __mla_file_system_native_get_fd_from_stream(input);

    if (fd == -1)
        return 0;

    ssize_t bytesRead = read(fd, buffer + offset, length);
    return bytesRead > 0 ? static_cast<mla_size_t>(bytesRead) : 0;
}

mla_size_t __mla_file_system_native_open_file_read_count(mla_size_t length) {

    const mla_size_t minimumParallelReadSize = 4u * 1024u * 1024u;
    const mla_size_t targetChunkSize = 1024u * 1024u;
    const mla_size_t minimumOutstandingReads = 4u;
    const mla_size_t maxOutstandingReads = 16u;

    if (length < minimumParallelReadSize) {
        return 1;
    }

    mla_size_t readCount = (length + targetChunkSize - 1u) / targetChunkSize;
    if (readCount < minimumOutstandingReads) {
        readCount = minimumOutstandingReads;
    }
    if (readCount > maxOutstandingReads) {
        readCount = maxOutstandingReads;
    }

    return readCount;
}

mla_size_t __mla_file_system_native_open_file_read_parallel(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {

    const mla_size_t maxOutstandingReads = 16u;

    int fd = __mla_file_system_native_get_fd_from_stream(input);

    if (fd == -1) {
        return 0;
    }

    mla_size_t readCount = __mla_file_system_native_open_file_read_count(length);
    if (readCount <= 1u) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    off_t currentPosition = lseek(fd, 0, SEEK_CUR);
    if (currentPosition == (off_t)-1) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    pthread_t threads[maxOutstandingReads] = {};
    mla_file_system_linux_parallel_read_task_t tasks[maxOutstandingReads] = {};

    mla_size_t createdThreadCount = 0;
    mla_size_t bytesScheduled = 0;
    const mla_size_t baseChunkSize = length / readCount;
    const mla_size_t remainder = length % readCount;

    for (mla_size_t i = 0; i < readCount; ++i) {
        const mla_size_t chunkSize = baseChunkSize + (i < remainder ? 1u : 0u);
        if (chunkSize == 0u) {
            continue;
        }

        tasks[i] = {
            fd,
            currentPosition + static_cast<off_t>(bytesScheduled),
            buffer + offset + bytesScheduled,
            chunkSize,
            0,
            false
        };

        if (pthread_create(&threads[i], nullptr, __mla_file_system_native_open_file_read_worker, &tasks[i]) != 0) {
            for (mla_size_t j = 0; j < createdThreadCount; ++j) {
                pthread_join(threads[j], nullptr);
            }
            return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
        }

        ++createdThreadCount;
        bytesScheduled += chunkSize;
    }

    mla_size_t totalBytesRead = 0;
    mla_bool_t hasFailure = false;
    for (mla_size_t i = 0; i < createdThreadCount; ++i) {
        pthread_join(threads[i], nullptr);
        if (tasks[i].failed) {
            hasFailure = true;
        }
        totalBytesRead += tasks[i].bytesRead;
    }

    if (hasFailure) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    if (lseek(fd, currentPosition + static_cast<off_t>(totalBytesRead), SEEK_SET) == (off_t)-1) {
        return __mla_file_system_native_open_file_read_sync(input, offset, length, buffer);
    }

    return totalBytesRead;
}

mla_bool_t __mla_file_system_native_open_file_seek(const mla_file_system_stream_t& stream, mla_size_t offset) {

    int fd = __mla_file_system_native_get_fd_from_stream(stream);

    if (fd == -1)
        return false;

    return lseek(fd, offset, SEEK_SET) != -1;
}

mla_size_t __mla_file_system_native_open_file_position(const mla_file_system_stream_t& stream) {

    int fd = __mla_file_system_native_get_fd_from_stream(stream);

    if (fd == -1)
        return 0;

    off_t pos = lseek(fd, 0, SEEK_CUR);
    return pos != -1 ? static_cast<mla_size_t>(pos) : 0;
}

mla_size_t __mla_file_system_native_open_file_length(const mla_file_system_stream_t& stream) {

    int fd = __mla_file_system_native_get_fd_from_stream(stream);

    if (fd == -1)
        return 0;

    // Save current position
    off_t currentPos = lseek(fd, 0, SEEK_CUR);
    if (currentPos == -1) return 0;

    // Seek to end and get position
    off_t size = lseek(fd, 0, SEEK_END);

    // Restore original position
    lseek(fd, currentPos, SEEK_SET);

    return size != -1 ? static_cast<mla_size_t>(size) : 0;
}

mla_bool_t __mla_file_system_native_open_file_set_length(const mla_file_system_stream_t& stream, mla_size_t length) {

    int fd = __mla_file_system_native_get_fd_from_stream(stream);

    if (fd == -1)
        return false;

    return ftruncate(fd, length) == 0;
}

mla_size_t __mla_file_system_native_open_file_read(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer) {
    return __mla_file_system_native_open_file_read_parallel(input, offset, length, buffer);
}

mla_size_t __mla_file_system_native_open_file_write(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {

    int fd = __mla_file_system_native_get_fd_from_stream(output);

    if (fd == -1)
        return 0;

    ssize_t bytesWritten = write(fd, buffer + offset, length);
    return bytesWritten > 0 ? static_cast<mla_size_t>(bytesWritten) : 0;
}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream) {

    mla_file_system_native_t* fs = __mla_file_system_native_get_native_data(file_system);

    if (fs == nullptr)
        return false;

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
    mla_string_destroy(fullPath);

    if (fd == -1)
        return false;

    // Allocate memory to store the file descriptor (freed in cleanup hook)
    int* fdPtr = static_cast<int*>(mla_platform_malloc(sizeof(int)));
    if (fdPtr == nullptr) {
        close(fd);
        return false;
    }

    *fdPtr = fd;

    mla_native_resource_t resource = mla_dynamic_data_from_pointer(fdPtr);
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
            path,
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
        close(fd);
        mla_platform_free(fdPtr);
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
//
// Created by chris on 10/13/2025.
//

#ifndef MLA_FILE_SYSTEM_H
#define MLA_FILE_SYSTEM_H

#include "../mla_data_types.h"
#include "../system/mla_buffer.h"
#include "../system/mla_stream.h"
#include "../system/mla_string.h"
#include "../system/mla_array_list.h"

enum mla_file_system_file_open_mode: mla_uint8_t {
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ,
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_WRITE,
    MLA_FILE_SYSTEM_FILE_OPEN_MODE_READ_AND_WRITE
};

struct mla_file_system_stream_t {
    mla_string_t path;

    mla_bool_t (*seek)(const mla_file_system_stream_t& stream, mla_size_t offset);
    mla_size_t (*position)(const mla_file_system_stream_t& stream);
    mla_size_t (*length)(const mla_file_system_stream_t& stream);
    mla_bool_t (*set_length)(const mla_file_system_stream_t& stream, mla_size_t length); // Optional, can be nullptr if not supported

    mla_size_t (*read)(const mla_file_system_stream_t& input, mla_size_t offset, mla_size_t length, mla_byte_t* buffer); // Optional, can be nullptr if not supported
    mla_size_t (*write)(const mla_file_system_stream_t& output, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer); // Optional, can be nullptr if not supported

    mla_dynamic_data_t resource;
    mla_buffer_reference_t resourceOwner;
};



/**
 * @brief Virtual file system interface for mounting custom file system implementations.
 *
 * This structure defines a set of function pointers that provide file and directory
 * operations for a mounted file system. Each file system implementation must provide
 * these callbacks to integrate with the file system manager.
 *
 * All paths passed to these callbacks are relative to the mount point and have been
 * normalized (lowercase). The file system manager handles mount point resolution
 * before invoking these callbacks.
 *
 * @note All callback functions should return `true` on success, `false` on failure.
 * @note Callbacks can be set to `nullptr` if the operation is not supported.
 */
struct mla_file_system_t {

    mla_bool_t (*file_exists)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*open_file)(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream);
    mla_bool_t (*delete_file)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*list_files)(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

    mla_bool_t (*create_directory)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*directory_exists)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*delete_directory)(mla_file_system_t& file_system, const mla_string_t& path);
    mla_bool_t (*list_directory)(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

    mla_dynamic_data_t user_data;
    mla_buffer_reference_t resourceOwner;
};

mla_file_system_stream_t mla_file_system_stream_empty();
mla_file_system_t mla_file_system_empty();

mla_bool_t mla_file_system_initialize(const mla_string_t& mount_path, const mla_file_system_t& file_system);
mla_bool_t mla_file_system_deinitialize(const mla_string_t& mount_path);

void mla_file_system_lock();
void mla_file_system_unlock();
mla_bool_t mla_file_system_is_locked();

//// File operations

mla_bool_t mla_fs_file_exists(const mla_string_t& path);
mla_bool_t mla_fs_delete_file(const mla_string_t& path);
mla_bool_t mla_fs_list_files(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

mla_bool_t mla_fs_open_file(const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_stream_t& out_stream);

//// Directory operations
mla_bool_t mla_fs_directory_exists(const mla_string_t& path);
mla_bool_t mla_fs_create_directory(const mla_string_t& path);
mla_bool_t mla_fs_delete_directory(const mla_string_t& path);
mla_bool_t mla_fs_list_directory(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

// Helpers
mla_bool_t mla_fs_is_directory_path(const mla_string_t& path);
mla_string_t mla_fs_get_parent_directory(const mla_string_t& path);
mla_string_t mla_fs_get_file_name(const mla_string_t& path);
mla_string_t mla_fs_get_file_extension(const mla_string_t& path);
mla_string_t mla_fs_change_file_extension(const mla_string_t& path, const mla_string_t& new_extension);
mla_string_t mla_fs_combine_paths(const mla_string_t& path1, const mla_string_t& path2);

/// Const
static const mla_string_t mla_fs_directory_seperator = mla_string_const("/");
static const mla_string_t mla_fs_root_directory = mla_fs_directory_seperator;

#endif
//
// Created by chris on 10/13/2025.
//

#ifndef COREOS_MLA_FILE_SYSTEM_H
#define COREOS_MLA_FILE_SYSTEM_H

#include "../mla_data_types.h"
#include "../system/mla_stream.h"
#include "../system/mla_string.h"
#include "../system/mla_array_list.h"

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

    mla_bool_t (*file_exists)(const mla_string_t& path);
    mla_bool_t (*open_file_for_write)(const mla_string_t& path, mla_stream_output_t& out_stream);
    mla_bool_t (*open_file_for_read)(const mla_string_t& path, mla_stream_input_t& out_stream);
    mla_bool_t (*delete_file)(const mla_string_t& path);
    mla_bool_t (*list_files)(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

    mla_bool_t (*create_directory)(const mla_string_t& path);
    mla_bool_t (*directory_exists)(const mla_string_t& path);
    mla_bool_t (*delete_directory)(const mla_string_t& path);
    mla_bool_t (*list_directory)(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);
};

mla_file_system_t mla_mla_file_system_empty();

mla_bool_t mla_file_system_initialize(const mla_string_t& mount_path, const mla_file_system_t& file_system);
mla_bool_t mla_file_system_deinitialize(const mla_string_t& mount_path);

void mla_file_system_lock();
void mla_file_system_unlock();
mla_bool_t mla_file_system_is_locked();

//// File operations

mla_bool_t mla_file_exists(const mla_string_t& path);
mla_bool_t mla_open_file_for_write(const mla_string_t& path, mla_stream_output_t& out_stream);
mla_bool_t mla_open_file_for_read(const mla_string_t& path, mla_stream_input_t& out_stream);
mla_bool_t mla_delete_file(const mla_string_t& path);
mla_bool_t mla_list_files(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

//// Directory operations
mla_bool_t mla_create_directory(const mla_string_t& path);
mla_bool_t mla_directory_exists(const mla_string_t& path);
mla_bool_t mla_delete_directory(const mla_string_t& path);
mla_bool_t mla_list_directory(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries);

/// Const
static const mla_string_t mla_directory_seperator = mla_string_const("/");

#endif
//
// Created by chris on 10/15/2025.
//

#ifndef COREOS_MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H
#define COREOS_MLA_GLOBAL_FILE_SYSTEM_WINDOWS_H

#include "../../core-os/filesystem/mla_file_system.h"
#include "../../core-os/mla_data_types.h"
#include "../../core-os/system/mla_string_concat.h"

struct mla_file_system_native_t {
    mla_string_t basePath;
};

mla_bool_t __mla_file_system_native_file_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);


}

mla_bool_t __mla_file_system_native_open_file(mla_file_system_t& file_system, const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_file_open_mode& out_stream) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_delete_file(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_list_files(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_create_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_directory_exists(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_delete_directory(mla_file_system_t& file_system, const mla_string_t& path) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
}

mla_bool_t __mla_file_system_native_list_directory(mla_file_system_t& file_system, const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_native_t* fs = reinterpret_cast<mla_file_system_native_t *>(file_system.user_data);
    mla_string_t fullPath = mla_string_concat(fs->basePath, path);
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
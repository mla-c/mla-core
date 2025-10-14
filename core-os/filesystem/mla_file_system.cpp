//
// Created by chris on 10/13/2025.
//

#include "mla_file_system.h"

#include "../task/mla_mutx.h"

mla_file_system_t mla_file_system_empty() {
    return { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, mla_buffer_reference_noOwner() };
}

mla_file_system_stream_t mla_file_system_stream_empty() {
    return { mla_string_empty(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, mla_buffer_reference_noOwner() };
}


struct mla_file_system_mount_t {
    mla_string_t mount_path;
    mla_file_system_t file_system;
};

struct mla_file_system_mount_initializer {
    static mla_file_system_mount_t init() {
        return { mla_string_empty(), mla_file_system_empty() };
    }
};

struct mla_file_system_manager_t {
    mla_bool_t locked;
    mla_mutex_t mounded_file_systems_mutex;
    mla_array_list_t<mla_file_system_mount_t, mla_file_system_mount_initializer> mounted_file_systems;
};



static mla_file_system_manager_t file_system_manager = {
    false,
    mla_mutex(mla_string_const("file_system_manager")),
    mla_array_list_empty<mla_file_system_mount_t, mla_file_system_mount_initializer>()
};

mla_int32_t __mla_file_system_sort_compare(const mla_file_system_mount_t &a, const mla_file_system_mount_t &b) {
    if (a.mount_path.length != b.mount_path.length) {
        return (a.mount_path.length > b.mount_path.length) ? -1 : 1;
    }
    return mla_string_compare(a.mount_path, b.mount_path);
}

void mla_file_system_lock() {
    if (file_system_manager.locked) {
        return;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return;
    }

    // We sort the list by the length desc and alphabetical order of the mount paths
    // This ensures that the most specific mount paths are checked first
    mla_array_list_sort(file_system_manager.mounted_file_systems, __mla_file_system_sort_compare);

    file_system_manager.locked = true;

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
}

void mla_file_system_unlock() {

    if (!file_system_manager.locked) {
        return;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return;
    }

    file_system_manager.locked = false;

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
}

mla_bool_t mla_file_system_is_locked() {

    return file_system_manager.locked;
}

mla_bool_t __mla_isvalid_directory_path(const mla_string_t& path) {

    // A valid directory path must start and end with a slash
    if (path.length < 2) {
        return false;
    }

    if (!mla_string_starts_with(path, mla_directory_seperator))
        return false;

    if (mla_string_ends_with(path, mla_directory_seperator))
        return false;

    return true;

}

mla_bool_t mla_file_system_initialize(const mla_string_t& mount_path, const mla_file_system_t& file_system) {

    if (!__mla_isvalid_directory_path(mount_path)) {
        mla_error(mla_string_concat("Mount point '", mount_path, "' is not a valid directory path. It must start and end with a slash."));
        return false;
    }


    if (file_system_manager.locked) {
        return false;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return false;
    }

    mla_string_t normalized_mount_path = mla_string_to_lower(mount_path);

    mla_bool_t result = mla_array_list_add(file_system_manager.mounted_file_systems, {
                                               mount_path,
                                               file_system
                                           });

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
    return result;
}

mla_bool_t mla_file_system_deinitialize(const mla_string_t &mount_path) {

    if (file_system_manager.locked) {
        return false;
    }

    if (!mla_mutex_lock(file_system_manager.mounded_file_systems_mutex)) {
        return false;
    }

    // Find the matching entity
    mla_int32_t index = -1;
    for (mla_size_t i = 0; i < mla_array_list_size(file_system_manager.mounted_file_systems); i++) {
        const mla_file_system_mount_t *mount = mla_array_list_get_ref(file_system_manager.mounted_file_systems, i);
        if (mla_string_equals_ignore_case(mount->mount_path, mount_path)) {
            index = i;
            break;
        }
    }

    mla_bool_t result = false;
    if (index != -1) {
        result = mla_array_list_remove(file_system_manager.mounted_file_systems, index);
    }

    mla_mutex_unlock(file_system_manager.mounded_file_systems_mutex);
    return result;
}

mla_bool_t __mla_find_file_system_for_path(const mla_string_t& path, mla_file_system_mount_t& out_file_system) {

    if (!file_system_manager.locked) {
        return false;
    }

    mla_string_t normalized_path = mla_string_to_lower(path);

    for (mla_size_t i = 0; i < mla_array_list_size(file_system_manager.mounted_file_systems); i++) {

        const mla_file_system_mount_t *mount = mla_array_list_get_ref(file_system_manager.mounted_file_systems, i);
        if (mla_string_starts_with(normalized_path, mount->mount_path)) {
            out_file_system = *mount;
            return true;
        }
    }

    return false;
}

mla_bool_t __mla_find_file_system_for_file(const mla_string_t& file_path, mla_file_system_mount_t& out_file_system) {

    // Split the in the file path into directory and file name
    mla_size_t last_slash_index = mla_string_last_index_of(file_path, mla_directory_seperator);
    if (last_slash_index == (mla_size_t)-1) {
        return false;
    }

    mla_string_t dicrectory_path = mla_string_substr(file_path, 0, last_slash_index + 1);
    return __mla_find_file_system_for_path(dicrectory_path, out_file_system);
}

mla_string_t __mla_get_relative_path(const mla_string_t& full_path, const mla_string_t& mount_path) {

    mla_string_t path = mla_string_substr(full_path, mount_path.length, full_path.length - 1);
    return mla_string_to_lower(path);
}

mla_bool_t mla_fs_file_exists(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.file_exists == nullptr) {
        return false;
    }

    return file_system_mount.file_system.file_exists(file_system_mount.file_system, relative_path);

}


mla_bool_t mla_fs_delete_file(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.delete_file == nullptr) {
        return false;
    }

    return file_system_mount.file_system.delete_file(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_fs_list_files(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.list_files == nullptr) {
        return false;
    }

    return file_system_mount.file_system.list_files(file_system_mount.file_system, relative_path, out_entries);

}

mla_bool_t mla_fs_create_directory(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.create_directory == nullptr) {
        return false;
    }

    return file_system_mount.file_system.create_directory(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_fs_directory_exists(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.directory_exists == nullptr) {
        return false;
    }

    return file_system_mount.file_system.directory_exists(file_system_mount.file_system, relative_path);

}

mla_bool_t mla_fs_delete_directory(const mla_string_t& path) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.delete_directory == nullptr) {
        return false;
    }

    return file_system_mount.file_system.delete_directory(file_system_mount.file_system, relative_path);
}

mla_bool_t mla_fs_list_directory(const mla_string_t& path, mla_array_list_t<mla_string_t, mla_string_initializer>& out_entries) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_path(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.list_directory == nullptr) {
        return false;
    }

    return file_system_mount.file_system.list_directory(file_system_mount.file_system, relative_path, out_entries);
}

mla_bool_t mla_fs_open_file(const mla_string_t& path, mla_file_system_file_open_mode mode, mla_file_system_file_open_mode& out_stream) {

    mla_file_system_mount_t file_system_mount = mla_file_system_mount_initializer::init();

    if (!__mla_find_file_system_for_file(path, file_system_mount)) {
        return false;
    }

    mla_string_t relative_path = __mla_get_relative_path(path, file_system_mount.mount_path);

    if (file_system_mount.file_system.open_file == nullptr) {
        return false;
    }

    return file_system_mount.file_system.open_file(file_system_mount.file_system, relative_path, mode, out_stream);
}

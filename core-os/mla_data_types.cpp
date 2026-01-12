//
// Created by christian on 1/11/26.
//

#include "mla_data_types.h"

const mla_char_t* mla_find_filename_from_path(const mla_char_t* path) {

    const mla_char_t* last = nullptr;

    while (*path != '\0') {
        // Check for both '/' and '\' as path separators because we dont not know the OS which has procedure the birnary
        if (*path == '/' || *path == '\\') {
            last = path;
        }
        path++;
    }

    if (last != nullptr) {
        return last + 1;
    }

    return path;

}

mla_pointer_t mla_malloc_with_check(mla_size_t size, const mla_char_t* path, const mla_char_t* function_name) {

    mla_pointer_t ptr = g_low_level_access.malloc(size);
    if (ptr == nullptr) {

        const mla_char_t* fileName =  mla_find_filename_from_path(path);
        g_low_level_access.on_malloc_failure(size, fileName, function_name);
    }
    return ptr;
}
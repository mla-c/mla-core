//
// Created by christian on 1/12/26.
//

#include "mla_test_data_types.h"

const mla_test_char_t* mla_test_find_filename_from_path(const mla_test_char_t* path) {

    const mla_test_char_t* last = nullptr;

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
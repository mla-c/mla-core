//
// Created by christian on 1/11/26.
//

#include "mla_logging.h"

void mla_private_logging_concat(mla_char_t* targetBuffer, const mla_char_t* filename, const mla_char_t* function) {

    const mla_size_t fn_len = mla_strlen(filename);
    const mla_size_t func_len = mla_strlen(function);
    const mla_size_t total_len = fn_len + 2 + func_len;
    mla_size_t offset = 0;
    if (total_len < 64) {
        mla_memcpy(targetBuffer + offset, filename, fn_len);
        offset += fn_len;
        targetBuffer[offset++] = ':';
        targetBuffer[offset++] = ':';
        mla_memcpy(targetBuffer + offset, function, func_len);
        targetBuffer[offset + func_len] = '\0';
    } else {
        const mla_size_t max_fn = mla_min(fn_len, 30);
        const mla_size_t max_func = mla_min(func_len, 31);
        mla_memcpy(targetBuffer, filename, max_fn);
        offset = max_fn;
        targetBuffer[offset++] = ':';
        targetBuffer[offset++] = ':';
        mla_memcpy(targetBuffer + offset, function, max_func);
        targetBuffer[offset + max_func] = '\0';
    }

}
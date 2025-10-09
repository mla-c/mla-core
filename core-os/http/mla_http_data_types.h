//
// Created by christian on 10/8/25.
//

#ifndef COREOS_MLA_HTTP_DATA_TYPES_H
#define COREOS_MLA_HTTP_DATA_TYPES_H

enum mla_http_version: mla_uint8_t {
    MLA_HTTP_VERSION_1_0,
    MLA_HTTP_VERSION_1_1,
    MLA_HTTP_VERSION_2_0,
    MLA_HTTP_VERSION_3_0
};

#define mla_http_max_header_size 8192 // 8 KB

#endif

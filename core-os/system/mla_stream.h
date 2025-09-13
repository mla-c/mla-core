//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_STREAM_H
#define COREOS_MLA_STREAM_H

#include "../mla_data_types.h"
#include "../system/mla_reference.h"

struct mla_stream_input_t {

    mla_callback_userdata userdata;
    mla_size_t (*read)(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, mla_byte_t* buffer);
    mla_buffer_reference_t refOwner;
};

struct mla_stream_output_t {

    mla_callback_userdata userdata;
    mla_size_t (*write)(mla_callback_userdata userdata, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer);
    mla_buffer_reference_t refOwner;
};


mla_stream_input_t mla_stream_noop_input();
mla_stream_output_t mla_stream_noop_output();

mla_stream_input_t mla_stream_input_from_buffer(mla_byte_t* buffer, mla_size_t size);
mla_stream_output_t mla_stream_output_to_buffer(mla_byte_t* buffer, mla_size_t size);

mla_stream_input_t mla_stream_input_from_buffer(mla_size_t size);
mla_stream_output_t mla_stream_output_to_buffer(mla_size_t size);


#endif

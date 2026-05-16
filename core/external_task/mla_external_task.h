//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_EXTERNAL_TASK_H
#define MLA_EXTERNAL_TASK_H

#include "../mla_data_types.h"
#include "../system/mla_stream.h"

struct mla_external_task_t {
    mla_native_resource_t native_resource;
    mla_string_t cmdline;
    mla_stream_input_t std_out;
    mla_stream_output_t std_in;
};

enum mla_external_task_state: mla_uint8_t {
    MLA_EXTERNAL_TASK_STATE_STOPPED,
    MLA_EXTERNAL_TASK_STATE_RUNNING,
};

mla_external_task_t mla_external_task_invalid();
mla_external_task_t mla_external_task_create(const mla_string_t& p_CmdLine);
mla_external_task_state mla_external_task_get_state(const mla_external_task_t& p_Task);
void mla_external_task_stop(mla_external_task_t& p_Task);

struct mla_external_task_managment_t {
    mla_bool_t (*create_process)(mla_native_resource_t& p_OutNativeResource, const mla_string_t& p_CmdLine);
    void (*stop_process)(const mla_native_resource_t& p_NativeResource);
    mla_external_task_state (*get_state)(const mla_native_resource_t& p_NativeResource);
    mla_size_t (*read_stdout)(const mla_native_resource_t& p_NativeResource, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer);
    mla_size_t (*write_stdin)(const mla_native_resource_t& p_NativeResource, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer);
};

mla_global mla_external_task_managment_t g_external_task_management;

#endif

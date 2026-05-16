//
// Created by Copilot on 5/15/2026.
//

#include "mla_external_task.h"

mla_global mla_external_task_managment_t g_external_task_management;

mla_user_data_id_init(mla_external_task_native_resource_user_data_name)

void __mla_external_task_cleanup(const mla_native_resource_t& p_Resource) {
    if (g_external_task_management.stop_process == nullptr) {
        return;
    }

    g_external_task_management.stop_process(p_Resource);
}

mla_size_t __mla_external_task_read_stdout(mla_stream_input_t& p_Input, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    if (g_external_task_management.read_stdout == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_native_resource_t nativeResource = mla_user_data_get_native_resource(p_Input.userdata, mla_external_task_native_resource_user_data_name);

    if (nativeResource.asPointer == nullptr) {
        return 0;
    }

    return g_external_task_management.read_stdout(nativeResource, p_Offset, p_Length, p_Buffer);
}

mla_size_t __mla_external_task_write_stdin(mla_stream_output_t& p_Output, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    if (g_external_task_management.write_stdin == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_native_resource_t nativeResource = mla_user_data_get_native_resource(p_Output.userdata, mla_external_task_native_resource_user_data_name);

    if (nativeResource.asPointer == nullptr) {
        return 0;
    }

    return g_external_task_management.write_stdin(nativeResource, p_Offset, p_Length, p_Buffer);
}

mla_external_task_t mla_external_task_invalid() {
    return {
        mla_native_resource_empty(),
        mla_string_empty(),
        mla_stream_noop_input(),
        mla_stream_noop_output()
    };
}

mla_external_task_t mla_external_task_create(const mla_string_t& p_CmdLine) {

    if (mla_string_is_empty(p_CmdLine)) {
        return mla_external_task_invalid();
    }

    if (g_external_task_management.create_process == nullptr) {
        return mla_external_task_invalid();
    }

    mla_external_task_t result = mla_external_task_invalid();
    if (!g_external_task_management.create_process(result.native_resource, p_CmdLine)) {
        return mla_external_task_invalid();
    }

    result.cmdline = mla_string_copy(p_CmdLine);

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_native_resource(userData, mla_external_task_native_resource_user_data_name, result.native_resource, __mla_external_task_cleanup);

    result.std_out = {
        userData,
        __mla_external_task_read_stdout,
        nullptr
    };

    result.std_in = {
        userData,
        __mla_external_task_write_stdin,
        nullptr
    };

    return result;
}

mla_external_task_state mla_external_task_get_state(const mla_external_task_t& p_Task) {

    if (p_Task.native_resource.asPointer == nullptr) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    if (g_external_task_management.get_state == nullptr) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    return g_external_task_management.get_state(p_Task.native_resource);
}

void mla_external_task_stop(mla_external_task_t& p_Task) {

    p_Task.std_out = mla_stream_noop_input();
    p_Task.std_in = mla_stream_noop_output();
    p_Task.native_resource = mla_native_resource_empty();
}

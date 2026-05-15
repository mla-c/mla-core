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

mla_native_resource_t* __mla_external_task_get_native_resource(const mla_user_data_t& p_UserData) {

    mla_pointer_t resourceOwner = mla_user_data_get_pointer(p_UserData, mla_external_task_native_resource_user_data_name);

    if (mla_pointer_is_null(resourceOwner)) {
        return nullptr;
    }

    return mla_native_resource_from_managed_pointer(resourceOwner);
}

mla_size_t __mla_external_task_read_stdout(mla_stream_input_t& p_Input, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    if (g_external_task_management.read_stdout == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_native_resource_t* nativeResource = __mla_external_task_get_native_resource(p_Input.userdata);

    if (nativeResource == nullptr) {
        return 0;
    }

    return g_external_task_management.read_stdout(*nativeResource, p_Offset, p_Length, p_Buffer);
}

mla_size_t __mla_external_task_write_stdin(mla_stream_output_t& p_Output, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    if (g_external_task_management.write_stdin == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    mla_native_resource_t* nativeResource = __mla_external_task_get_native_resource(p_Output.userdata);

    if (nativeResource == nullptr) {
        return 0;
    }

    return g_external_task_management.write_stdin(*nativeResource, p_Offset, p_Length, p_Buffer);
}

mla_external_task_t mla_external_task_invalid() {
    return {
        mla_native_resource_empty(),
        mla_string_empty(),
        mla_stream_noop_input(),
        mla_stream_noop_output(),
        mla_pointer_null()
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
    result.cmdline = mla_string_copy(p_CmdLine);

    if (!g_external_task_management.create_process(result.native_resource, p_CmdLine)) {
        return mla_external_task_invalid();
    }

    result.native_resource_owner = mla_native_resource_to_managed_pointer(result.native_resource, __mla_external_task_cleanup);

    if (mla_pointer_is_null(result.native_resource_owner)) {
        __mla_external_task_cleanup(result.native_resource);
        return mla_external_task_invalid();
    }

    mla_user_data_t userData = mla_user_data_empty();
    mla_user_data_set_pointer(userData, mla_external_task_native_resource_user_data_name, result.native_resource_owner);

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

void mla_external_task_stop(mla_external_task_t& p_Task) {

    p_Task.native_resource_owner = mla_pointer_null();
    p_Task.native_resource = mla_native_resource_empty();
    p_Task.std_out = mla_stream_noop_input();
    p_Task.std_in = mla_stream_noop_output();
}

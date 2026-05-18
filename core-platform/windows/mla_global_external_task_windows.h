//
// Created by Copilot on 5/16/2026.
//

#ifndef MLA_GLOBAL_EXTERNAL_TASK_WINDOWS_H
#define MLA_GLOBAL_EXTERNAL_TASK_WINDOWS_H

#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string.h"
#include "../../core/mla_native_resource.h"

#include <windows.h>

struct __windows_external_task_native_resource_t {
    HANDLE process_handle;
    HANDLE thread_handle;
    HANDLE stdin_write_handle;
    HANDLE stdout_read_handle;

    static void clean_up_resource(__windows_external_task_native_resource_t* self) {

        if (self == nullptr) {
            return;
        }

        if (self->stdin_write_handle != nullptr) {
            CloseHandle(self->stdin_write_handle);
            self->stdin_write_handle = nullptr;
        }

        if (self->stdout_read_handle != nullptr) {
            CloseHandle(self->stdout_read_handle);
            self->stdout_read_handle = nullptr;
        }

        if (self->process_handle != nullptr) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(self->process_handle, &exitCode) && exitCode == STILL_ACTIVE) {
                TerminateProcess(self->process_handle, 1);
                WaitForSingleObject(self->process_handle, INFINITE);
            }
        }

        if (self->thread_handle != nullptr) {
            CloseHandle(self->thread_handle);
            self->thread_handle = nullptr;
        }

        if (self->process_handle != nullptr) {
            CloseHandle(self->process_handle);
            self->process_handle = nullptr;
        }
    }
};

void __windows_external_task_cleanup_process_data(__windows_external_task_native_resource_t* p_ProcessData) {

    if (p_ProcessData == nullptr) {
        return;
    }

    if (p_ProcessData->stdin_write_handle != nullptr) {
        CloseHandle(p_ProcessData->stdin_write_handle);
        p_ProcessData->stdin_write_handle = nullptr;
    }

    if (p_ProcessData->stdout_read_handle != nullptr) {
        CloseHandle(p_ProcessData->stdout_read_handle);
        p_ProcessData->stdout_read_handle = nullptr;
    }
}

void __windows_external_task_cleanup_process_handles(__windows_external_task_native_resource_t* p_ProcessData) {

    if (p_ProcessData == nullptr) {
        return;
    }

    if (p_ProcessData->thread_handle != nullptr) {
        CloseHandle(p_ProcessData->thread_handle);
        p_ProcessData->thread_handle = nullptr;
    }

    if (p_ProcessData->process_handle != nullptr) {
        CloseHandle(p_ProcessData->process_handle);
        p_ProcessData->process_handle = nullptr;
    }
}

__windows_external_task_native_resource_t* __windows_external_task_get_process_data(const mla_pointer_t& p_TaskResource) {

    return mla_native_resource_struct_from_managed_pointer<__windows_external_task_native_resource_t>(p_TaskResource);
}

mla_bool_t __windows_external_task_create_process(mla_pointer_t& p_OutTaskResource, const mla_string_t& p_CmdLine) {

    p_OutTaskResource = mla_pointer_null();

    if (mla_string_is_empty(p_CmdLine)) {
        return false;
    }

    SECURITY_ATTRIBUTES securityAttributes = {};
    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE childStdOutRead = nullptr;
    HANDLE childStdOutWrite = nullptr;
    HANDLE childStdInRead = nullptr;
    HANDLE childStdInWrite = nullptr;

    if (!CreatePipe(&childStdOutRead, &childStdOutWrite, &securityAttributes, 0)) {
        return false;
    }

    if (!SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdOutWrite);
        return false;
    }

    if (!CreatePipe(&childStdInRead, &childStdInWrite, &securityAttributes, 0)) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdOutWrite);
        return false;
    }

    if (!SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdOutWrite);
        CloseHandle(childStdInRead);
        CloseHandle(childStdInWrite);
        return false;
    }

    // Set the stdin write handle to non-blocking mode so WriteFile returns immediately
    // when the pipe buffer is full instead of blocking.
    DWORD pipeMode = PIPE_NOWAIT | PIPE_READMODE_BYTE;
    SetNamedPipeHandleState(childStdInWrite, &pipeMode, nullptr, nullptr);

    mla_c_string_t cmdlineCStr = mla_string_to_cString(p_CmdLine);
    const mla_char_t* cmdline = mla_c_string_data(cmdlineCStr);
    if (cmdline == nullptr) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdOutWrite);
        CloseHandle(childStdInRead);
        CloseHandle(childStdInWrite);
        return false;
    }

    const mla_char_t* prefix = "cmd.exe /C \"";
    const mla_char_t* suffix = "\"";
    mla_size_t cmdlineLength = mla_strlen(cmdline);
    mla_size_t prefixLength = mla_strlen(prefix);
    mla_size_t suffixLength = mla_strlen(suffix);
    mla_size_t commandLength = prefixLength + cmdlineLength + suffixLength;

    mla_char_t* commandBuffer = static_cast<mla_char_t*>(mla_platform_malloc(commandLength + 1));
    if (commandBuffer == nullptr) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdOutWrite);
        CloseHandle(childStdInRead);
        CloseHandle(childStdInWrite);
        return false;
    }

    mla_memcpy(commandBuffer, prefix, prefixLength);
    mla_memcpy(commandBuffer + prefixLength, cmdline, cmdlineLength);
    mla_memcpy(commandBuffer + prefixLength + cmdlineLength, suffix, suffixLength);
    commandBuffer[commandLength] = '\0';

    STARTUPINFOA startupInfo = {};
    PROCESS_INFORMATION processInformation = {};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdError = childStdOutWrite;
    startupInfo.hStdOutput = childStdOutWrite;
    startupInfo.hStdInput = childStdInRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    BOOL processCreated = CreateProcessA(
        nullptr,
        commandBuffer,
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startupInfo,
        &processInformation
    );

    mla_platform_free(commandBuffer);
    CloseHandle(childStdOutWrite);
    CloseHandle(childStdInRead);

    if (!processCreated) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdInWrite);
        return false;
    }

    p_OutTaskResource = mla_malloc_native_resource_struct(__windows_external_task_native_resource_t);
    __windows_external_task_native_resource_t* processData = mla_native_resource_struct_from_managed_pointer<__windows_external_task_native_resource_t>(p_OutTaskResource);

    if (processData == nullptr) {
        CloseHandle(childStdOutRead);
        CloseHandle(childStdInWrite);
        TerminateProcess(processInformation.hProcess, 1);
        WaitForSingleObject(processInformation.hProcess, INFINITE);
        CloseHandle(processInformation.hThread);
        CloseHandle(processInformation.hProcess);
        return false;
    }

    processData->process_handle = processInformation.hProcess;
    processData->thread_handle = processInformation.hThread;
    processData->stdin_write_handle = childStdInWrite;
    processData->stdout_read_handle = childStdOutRead;


    return true;
}

mla_external_task_state __windows_external_task_get_state(const mla_pointer_t& p_TaskResource) {

    __windows_external_task_native_resource_t* processData = __windows_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->process_handle == nullptr) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    DWORD exitCode = 0;
    if (!GetExitCodeProcess(processData->process_handle, &exitCode)) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    if (exitCode == STILL_ACTIVE) {
        return MLA_EXTERNAL_TASK_STATE_RUNNING;
    }

    __windows_external_task_cleanup_process_data(processData);
    __windows_external_task_cleanup_process_handles(processData);
    return MLA_EXTERNAL_TASK_STATE_STOPPED;
}

void __windows_external_task_stop_process(const mla_pointer_t& p_TaskResource) {

    __windows_external_task_native_resource_t* processData = __windows_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    __windows_external_task_cleanup_process_data(processData);

    if (processData->process_handle != nullptr) {
        TerminateProcess(processData->process_handle, 1);
        WaitForSingleObject(processData->process_handle, INFINITE);
    }

    __windows_external_task_cleanup_process_handles(processData);
}

mla_size_t __windows_external_task_read_stdout(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    __windows_external_task_native_resource_t* processData = __windows_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdout_read_handle == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: peek first to see how many bytes are available.
    // If none are available we return immediately without blocking.
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(processData->stdout_read_handle, nullptr, 0, nullptr, &bytesAvailable, nullptr)) {
        return 0;
    }

    if (bytesAvailable == 0) {
        return 0;
    }

    DWORD toRead = static_cast<DWORD>(bytesAvailable < static_cast<DWORD>(p_Length) ? bytesAvailable : static_cast<DWORD>(p_Length));

    DWORD bytesRead = 0;
    if (!ReadFile(processData->stdout_read_handle, p_Buffer + p_Offset, toRead, &bytesRead, nullptr)) {
        return 0;
    }

    return static_cast<mla_size_t>(bytesRead);
}

mla_size_t __windows_external_task_write_stdin(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    __windows_external_task_native_resource_t* processData = __windows_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdin_write_handle == nullptr || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: the write handle was set to PIPE_NOWAIT so WriteFile returns
    // immediately with ERROR_NO_DATA when the pipe buffer is full instead of blocking.
    DWORD bytesWritten = 0;
    if (!WriteFile(processData->stdin_write_handle, p_Buffer + p_Offset, static_cast<DWORD>(p_Length), &bytesWritten, nullptr)) {
        // ERROR_NO_DATA  — pipe buffer full, nothing written yet (non-blocking return)
        // Any other error — connection broken or similar
        return static_cast<mla_size_t>(bytesWritten);
    }

    return static_cast<mla_size_t>(bytesWritten);
}

void __windows_external_task_close_stdin(const mla_pointer_t& p_TaskResource) {

    __windows_external_task_native_resource_t* processData = __windows_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    if (processData->stdin_write_handle != nullptr) {
        CloseHandle(processData->stdin_write_handle);
        processData->stdin_write_handle = nullptr;
    }
}

mla_external_task_managment_t g_external_task_management = {
    __windows_external_task_create_process,
    __windows_external_task_stop_process,
    __windows_external_task_get_state,
    __windows_external_task_read_stdout,
    __windows_external_task_write_stdin,
    __windows_external_task_close_stdin,
};

#endif

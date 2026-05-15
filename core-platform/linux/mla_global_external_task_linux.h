//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_GLOBAL_EXTERNAL_TASK_LINUX_H
#define MLA_GLOBAL_EXTERNAL_TASK_LINUX_H

#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string.h"

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <spawn.h>

extern char** environ;

struct __linux_external_task_native_resource_t {
    pid_t pid;
    mla_int32_t stdin_write_fd;
    mla_int32_t stdout_read_fd;
};

void __linux_external_task_cleanup_process_data(__linux_external_task_native_resource_t* p_ProcessData) {

    if (p_ProcessData == nullptr) {
        return;
    }

    if (p_ProcessData->stdin_write_fd >= 0) {
        close(p_ProcessData->stdin_write_fd);
        p_ProcessData->stdin_write_fd = -1;
    }

    if (p_ProcessData->stdout_read_fd >= 0) {
        close(p_ProcessData->stdout_read_fd);
        p_ProcessData->stdout_read_fd = -1;
    }
}

mla_bool_t __linux_external_task_create_process(mla_native_resource_t& p_OutNativeResource, const mla_string_t& p_CmdLine) {

    p_OutNativeResource = mla_native_resource_empty();

    if (mla_string_is_empty(p_CmdLine)) {
        return false;
    }

    int stdinPipe[2] = {-1, -1};
    int stdoutPipe[2] = {-1, -1};

    if (pipe(stdinPipe) != 0) {
        return false;
    }

    if (pipe(stdoutPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return false;
    }

    mla_c_string_t cmdlineCStr = mla_string_to_cString(p_CmdLine);
    const mla_char_t* cmdline = mla_c_string_data(cmdlineCStr);

    if (cmdline == nullptr) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    posix_spawn_file_actions_t fileActions;
    if (posix_spawn_file_actions_init(&fileActions) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    if (posix_spawn_file_actions_adddup2(&fileActions, stdinPipe[0], STDIN_FILENO) != 0 ||
        posix_spawn_file_actions_adddup2(&fileActions, stdoutPipe[1], STDOUT_FILENO) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, stdinPipe[0]) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, stdinPipe[1]) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[0]) != 0 ||
        posix_spawn_file_actions_addclose(&fileActions, stdoutPipe[1]) != 0) {

        posix_spawn_file_actions_destroy(&fileActions);
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    mla_char_t* const argv[] = {
        const_cast<mla_char_t*>("sh"),
        const_cast<mla_char_t*>("-c"),
        const_cast<mla_char_t*>(cmdline),
        nullptr
    };

    pid_t pid = -1;
    int spawnResult = posix_spawn(&pid, "/bin/sh", &fileActions, nullptr, argv, environ);
    posix_spawn_file_actions_destroy(&fileActions);

    if (spawnResult != 0 || pid <= 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    __linux_external_task_native_resource_t* processData = static_cast<__linux_external_task_native_resource_t*>(mla_platform_malloc(sizeof(__linux_external_task_native_resource_t)));

    if (processData == nullptr) {
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        return false;
    }

    processData->pid = pid;
    processData->stdin_write_fd = stdinPipe[1];
    processData->stdout_read_fd = stdoutPipe[0];

    p_OutNativeResource = mla_dynamic_data_from_pointer(processData);
    return true;
}

void __linux_external_task_stop_process(const mla_native_resource_t& p_NativeResource) {

    __linux_external_task_native_resource_t* processData = static_cast<__linux_external_task_native_resource_t*>(p_NativeResource.asPointer);

    if (processData == nullptr) {
        return;
    }

    __linux_external_task_cleanup_process_data(processData);

    if (processData->pid > 0) {
        kill(processData->pid, SIGTERM);
        waitpid(processData->pid, nullptr, 0);
        processData->pid = -1;
    }

    mla_platform_free(processData);
}

mla_size_t __linux_external_task_read_stdout(const mla_native_resource_t& p_NativeResource, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    __linux_external_task_native_resource_t* processData = static_cast<__linux_external_task_native_resource_t*>(p_NativeResource.asPointer);

    if (processData == nullptr || processData->stdout_read_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    ssize_t bytesRead = read(processData->stdout_read_fd, p_Buffer + p_Offset, p_Length);

    if (bytesRead <= 0) {
        return 0;
    }

    return (mla_size_t)bytesRead;
}

mla_size_t __linux_external_task_write_stdin(const mla_native_resource_t& p_NativeResource, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    __linux_external_task_native_resource_t* processData = static_cast<__linux_external_task_native_resource_t*>(p_NativeResource.asPointer);

    if (processData == nullptr || processData->stdin_write_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    mla_size_t totalWritten = 0;

    while (totalWritten < p_Length) {

        ssize_t bytesWritten = write(processData->stdin_write_fd, p_Buffer + p_Offset + totalWritten, p_Length - totalWritten);

        if (bytesWritten <= 0) {
            break;
        }

        totalWritten += (mla_size_t)bytesWritten;
    }

    return totalWritten;
}

mla_external_task_managment_t g_external_task_management = {
    __linux_external_task_create_process,
    __linux_external_task_stop_process,
    __linux_external_task_read_stdout,
    __linux_external_task_write_stdin,
};

#endif

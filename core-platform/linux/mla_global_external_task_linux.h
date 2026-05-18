//
// Created by Copilot on 5/15/2026.
//

#ifndef MLA_GLOBAL_EXTERNAL_TASK_LINUX_H
#define MLA_GLOBAL_EXTERNAL_TASK_LINUX_H

#include "../../core/external_task/mla_external_task.h"
#include "../../core/system/mla_string.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>

struct __linux_external_task_native_resource_t {
    pid_t pid;
    mla_int32_t stdin_write_fd;
    mla_int32_t stdout_read_fd;
};

void __linux_external_task_child_fail(mla_int32_t p_StatusPipeFd) {
    mla_uint8_t childError = 1;
    write(p_StatusPipeFd, &childError, sizeof(childError));
    kill(getpid(), SIGKILL);
    while (true) {
        pause();
    }
}

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

__linux_external_task_native_resource_t* __linux_external_task_get_process_data(const mla_pointer_t& p_TaskResource) {

    mla_native_resource_t* nativeResource = mla_native_resource_from_managed_pointer(p_TaskResource);
    if (nativeResource == nullptr) {
        return nullptr;
    }

    return static_cast<__linux_external_task_native_resource_t*>(nativeResource->asPointer);
}

void __linux_external_task_cleanup_native_resource(const mla_native_resource_t& p_NativeResource) {

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

mla_bool_t __linux_external_task_create_process(mla_pointer_t& p_OutTaskResource, const mla_string_t& p_CmdLine) {

    p_OutTaskResource = mla_pointer_null();

    if (mla_string_is_empty(p_CmdLine)) {
        return false;
    }

    int stdinPipe[2] = {-1, -1};
    int stdoutPipe[2] = {-1, -1};
    int statusPipe[2] = {-1, -1};

    if (pipe(stdinPipe) != 0) {
        return false;
    }

    if (pipe(stdoutPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        return false;
    }

    if (pipe(statusPipe) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        return false;
    }

    if (fcntl(statusPipe[1], F_SETFD, FD_CLOEXEC) != 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(statusPipe[0]);
        close(statusPipe[1]);
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(stdinPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(statusPipe[0]);
        close(statusPipe[1]);
        return false;
    }

    if (pid == 0) {

        close(statusPipe[0]);
        close(stdinPipe[1]);
        close(stdoutPipe[0]);

        if (prctl(PR_SET_PDEATHSIG, SIGTERM) != 0 || getppid() == 1) {
            __linux_external_task_child_fail(statusPipe[1]);
        }

        if (dup2(stdinPipe[0], STDIN_FILENO) < 0 || dup2(stdoutPipe[1], STDOUT_FILENO) < 0) {
            __linux_external_task_child_fail(statusPipe[1]);
        }

        close(stdinPipe[0]);
        close(stdoutPipe[1]);

        mla_c_string_t cmdlineCStr = mla_string_to_cString(p_CmdLine);
        const mla_char_t* cmdline = mla_c_string_data(cmdlineCStr);

        if (cmdline == nullptr) {
            __linux_external_task_child_fail(statusPipe[1]);
        }

        execl("/bin/sh", "sh", "-c", cmdline, nullptr);
        __linux_external_task_child_fail(statusPipe[1]);
    }

    close(statusPipe[1]);
    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    mla_uint8_t childError = 0;
    ssize_t childState = 0;
    do {
        childState = read(statusPipe[0], &childError, sizeof(childError));
    } while (childState < 0 && errno == EINTR);
    close(statusPipe[0]);

    if (childState != 0) {
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        waitpid(pid, nullptr, 0);
        return false;
    }

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

    // Make both parent-side fds non-blocking so reads/writes return
    // immediately instead of blocking when no data / buffer is full.
    fcntl(processData->stdout_read_fd, F_SETFL, O_NONBLOCK);
    fcntl(processData->stdin_write_fd,  F_SETFL, O_NONBLOCK);

    mla_native_resource_t nativeResource = mla_dynamic_data_from_pointer(processData);
    p_OutTaskResource = mla_native_resource_to_managed_pointer(nativeResource, __linux_external_task_cleanup_native_resource);

    if (mla_pointer_is_null(p_OutTaskResource)) {
        __linux_external_task_cleanup_native_resource(nativeResource);
        return false;
    }

    return true;
}

mla_external_task_state __linux_external_task_get_state(const mla_pointer_t& p_TaskResource) {

    __linux_external_task_native_resource_t* processData = __linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->pid <= 0) {
        return MLA_EXTERNAL_TASK_STATE_STOPPED;
    }

    int status = 0;
    pid_t waitResult = -1;
    do {
        waitResult = waitpid(processData->pid, &status, WNOHANG);
    } while (waitResult < 0 && errno == EINTR);

    if (waitResult == 0) {
        return MLA_EXTERNAL_TASK_STATE_RUNNING;
    }

    processData->pid = -1;
    __linux_external_task_cleanup_process_data(processData);
    return MLA_EXTERNAL_TASK_STATE_STOPPED;
}

void __linux_external_task_stop_process(const mla_pointer_t& p_TaskResource) {

    __linux_external_task_native_resource_t* processData = __linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    __linux_external_task_cleanup_process_data(processData);

    if (processData->pid > 0) {
        kill(processData->pid, SIGTERM);
        waitpid(processData->pid, nullptr, 0);
        processData->pid = -1;
    }
}

mla_size_t __linux_external_task_read_stdout(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, mla_byte_t* p_Buffer) {

    __linux_external_task_native_resource_t* processData = __linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdout_read_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: fd has O_NONBLOCK set; EAGAIN/EWOULDBLOCK means no data yet.
    ssize_t bytesRead = read(processData->stdout_read_fd, p_Buffer + p_Offset, p_Length);

    if (bytesRead <= 0) {
        return 0;
    }

    return static_cast<mla_size_t>(bytesRead);
}

mla_size_t __linux_external_task_write_stdin(const mla_pointer_t& p_TaskResource, mla_size_t p_Offset, mla_size_t p_Length, const mla_byte_t* p_Buffer) {

    __linux_external_task_native_resource_t* processData = __linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr || processData->stdin_write_fd < 0 || p_Buffer == nullptr) {
        return 0;
    }

    // Non-blocking: fd has O_NONBLOCK set; EAGAIN/EWOULDBLOCK means pipe buffer full.
    ssize_t bytesWritten = write(processData->stdin_write_fd, p_Buffer + p_Offset, p_Length);

    if (bytesWritten <= 0) {
        return 0;
    }

    return static_cast<mla_size_t>(bytesWritten);
}

void __linux_external_task_close_stdin(const mla_pointer_t& p_TaskResource) {

    __linux_external_task_native_resource_t* processData = __linux_external_task_get_process_data(p_TaskResource);

    if (processData == nullptr) {
        return;
    }

    if (processData->stdin_write_fd >= 0) {
        close(processData->stdin_write_fd);
        processData->stdin_write_fd = -1;
    }
}

mla_external_task_managment_t g_external_task_management = {
    __linux_external_task_create_process,
    __linux_external_task_stop_process,
    __linux_external_task_get_state,
    __linux_external_task_read_stdout,
    __linux_external_task_write_stdin,
    __linux_external_task_close_stdin,
};

#endif

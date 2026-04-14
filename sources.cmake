# Core-OS library source files

# Main source files
set(SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/mla_data_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/mla_data_types_pointer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_string.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_string_concat.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_string_utf.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_string_convert.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_number.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_user_data.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/log/mla_logging.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/log/mla_logger_console.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/log/mla_logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/memory/mla_memory_hook.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_mutx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_rw_lock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_atomic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_task_local.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_task_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_task_cli_module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/task/mla_task_manager_data_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/cli/mla_cli_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/cli/mla_cli_app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_stream_helper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_stream_wrapper.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/system/mla_stream_deflate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/config/mla_config.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/serializer/mla_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/serializer/mla_binary_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/serializer/mla_json_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/serializer/mla_xml_serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/utils/mla_endian_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/utils/mla_math_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/hash/mla_sha1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/rpc/mla_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/lifecycle/mla_lifecycle_events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/log/mla_logger_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/reflection/mla_reflection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/reflection/mla_reflection_rpc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_control.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_label.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_button.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_text_edit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_loading_indicator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/surfaces/mla_ui_surface_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/controls/mla_ui_control_surface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/surfaces/mla_ui_surface.cpp
)

# Network source files
set(SOURCE_NETWORK_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_http_header.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_http_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_http_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_http_chunked_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_websocket_utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/http/mla_websocket_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/url/mla_url.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/network/mla_network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/rpc/mla_rpc_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/rpc/mla_rpc_http_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/web/mla_ui_http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/web/mla_ui_web_remote_surface.cpp
)

# File system source files
set(SOURCE_FILE_SYSTEM_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/filesystem/mla_file_system.cpp
)

# UI source files
set(SOURCE_UI_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os/ui/display/mla_ui_display_surface.cpp
)

# Test support source files
set(SOURCE_TEST_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/mla_test_data_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/Benchmark/mla_benchmark.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/Test/mla_test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/mla_benchmark_executor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/mla_test_executor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/core-os-test-support/mla_test_utils.cpp
)
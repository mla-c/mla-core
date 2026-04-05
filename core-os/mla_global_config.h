#ifndef COREOS_MLA_GLOBAL_CONFIG_H
#define COREOS_MLA_GLOBAL_CONFIG_H

/////////////////////////////////////////////////
// Configuration Parameters for the MLA framework
/////////////////////////////////////////////////

// Math Constants
#ifndef mla_math_pi
#define mla_math_pi 3.14159265358979323846
#endif

// Logger Configuration
#ifndef mla_rpc_log_cache_size
#define mla_rpc_log_cache_size 128
#endif

#ifndef mla_logging_use_native
#define mla_logging_use_native 0
#endif

#ifndef mla_logger_console_level
#define mla_logger_console_level MLA_LOG_LEVEL_INFO
#endif

// String SSO Configuration
#ifndef mla_string_sso_max_length
#define mla_string_sso_max_length 14
#endif

// Data Structure Defaults
#ifndef mla_array_list_default_capacity
#define mla_array_list_default_capacity 8
#endif

#ifndef CONST_mla_hash_map_item_default_size
#define CONST_mla_hash_map_item_default_size 2
#endif

#ifndef CONST_mla_hash_map_default_bucket_size
#define CONST_mla_hash_map_default_bucket_size 8
#endif

#ifndef CONST_mla_hash_map_default_load_factor
#define CONST_mla_hash_map_default_load_factor 0.75
#endif

// Stream Configuration
#ifndef mla_stream_fast_read_buffer_size
#define mla_stream_fast_read_buffer_size 128
#endif

#ifndef mla_stream_output_deflate_min_compression_data_size
#define mla_stream_output_deflate_min_compression_data_size 1024
#endif

// HTTP Configuration
#ifndef mla_default_http_timeout_ms
#define mla_default_http_timeout_ms 30000
#endif

#ifndef mla_http_use_deflate_compression
#define mla_http_use_deflate_compression 1
#endif

#ifndef mla_http_max_header_size
#define mla_http_max_header_size 8192 // 8 KB
#endif

#ifndef mla_http_server_use_deflate_compression
#define mla_http_server_use_deflate_compression mla_http_use_deflate_compression
#endif

#ifndef mla_http_client_use_deflate_compression
#define mla_http_client_use_deflate_compression mla_http_use_deflate_compression
#endif

#ifndef mla_http_client_use_gzip_compression
#define mla_http_client_use_gzip_compression mla_http_use_deflate_compression
#endif

// WebSocket Configuration
#ifndef mla_websocket_close_normal
#define mla_websocket_close_normal 1000
#endif

#ifndef mla_websocket_close_abnormal
#define mla_websocket_close_abnormal 1006
#endif

// Task and Lock Configuration
#ifndef mla_mutex_default_wait_timeout
#define mla_mutex_default_wait_timeout 1000 // Default timeout for mutex lock in milliseconds
#endif

#ifndef mla_rw_lock_default_timeout
#define mla_rw_lock_default_timeout 1000 // Default timeout for read/write locks in milliseconds
#endif

// Memory Hook Configuration
#ifndef CONST_MLA_MAX_MEMORY_HOOKS
#define CONST_MLA_MAX_MEMORY_HOOKS 10
#endif

// RPC Configuration
#ifndef mla_rpc_stream_small_buffer_size
#define mla_rpc_stream_small_buffer_size 1024 // 1 KB
#endif

#endif // COREOS_MLA_GLOBAL_CONFIG_H

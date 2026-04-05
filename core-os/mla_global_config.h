#ifndef COREOS_MLA_GLOBAL_CONFIG_H
#define COREOS_MLA_GLOBAL_CONFIG_H

/**
 * @file mla_global_config.h
 * @brief Global configuration parameters for the MLA framework.
 *
 * This file contains default values for various framework settings.
 * All parameters are wrapped in #ifndef guards, allowing them to be
 * overridden at compile-time via compiler flags (e.g., -Dmla_string_sso_max_length=16).
 */

/////////////////////////////////////////////////
// Logger Configuration
/////////////////////////////////////////////////

/**
 * @brief Size of the log message cache for the RPC logger.
 * Defines how many recent log entries are kept in memory for retrieval via RPC.
 */
#ifndef mla_rpc_log_cache_size
#define mla_rpc_log_cache_size 128
#endif

/**
 * @brief Flag to enable native logging output.
 * If set to 1, log messages are printed directly to the system output using mla_print.
 */
#ifndef mla_logging_use_native
#define mla_logging_use_native 0
#endif

/**
 * @brief Default log level for the console logger.
 * Messages with a priority lower than this level will be filtered out.
 */
#ifndef mla_logger_console_level
#define mla_logger_console_level MLA_LOG_LEVEL_INFO
#endif

/////////////////////////////////////////////////
// String Configuration
/////////////////////////////////////////////////

/**
 * @brief Maximum length for Small String Optimization (SSO).
 * Strings up to this length (in bytes) are stored within the string structure itself,
 * avoiding heap allocation.
 */
#ifndef mla_string_sso_max_length
#define mla_string_sso_max_length 14
#endif

/////////////////////////////////////////////////
// Data Structure Defaults
/////////////////////////////////////////////////

/**
 * @brief Default initial capacity for mla_array_list_t.
 */
#ifndef mla_array_list_default_capacity
#define mla_array_list_default_capacity 8
#endif

/**
 * @brief Default initial size for items list within a hash map bucket.
 */
#ifndef CONST_mla_hash_map_item_default_size
#define CONST_mla_hash_map_item_default_size 2
#endif

/**
 * @brief Default number of buckets for a new hash map.
 */
#ifndef CONST_mla_hash_map_default_bucket_size
#define CONST_mla_hash_map_default_bucket_size 8
#endif

/**
 * @brief Default load factor for hash map rebalancing.
 * When the ratio of items to buckets exceeds this value, the map is resized.
 */
#ifndef CONST_mla_hash_map_default_load_factor
#define CONST_mla_hash_map_default_load_factor 0.75
#endif

/////////////////////////////////////////////////
// Stream Configuration
/////////////////////////////////////////////////

/**
 * @brief Buffer size for high-speed or non-blocking stream reads.
 * Used to balance memory usage and I/O efficiency.
 */
#ifndef mla_stream_fast_read_buffer_size
#define mla_stream_fast_read_buffer_size 128
#endif

/**
 * @brief Minimum data size required to trigger DEFLATE compression.
 * Smaller data may result in a larger compressed output or inefficient processing.
 */
#ifndef mla_stream_output_deflate_min_compression_data_size
#define mla_stream_output_deflate_min_compression_data_size 1024
#endif

/////////////////////////////////////////////////
// HTTP Configuration
/////////////////////////////////////////////////

/**
 * @brief Default timeout in milliseconds for HTTP client and server operations.
 */
#ifndef mla_default_http_timeout_ms
#define mla_default_http_timeout_ms 30000
#endif

/**
 * @brief Global toggle for DEFLATE compression support in HTTP modules.
 */
#ifndef mla_http_use_deflate_compression
#define mla_http_use_deflate_compression 1
#endif

/**
 * @brief Maximum permitted size for HTTP headers in bytes.
 */
#ifndef mla_http_max_header_size
#define mla_http_max_header_size 8192 // 8 KB
#endif

/**
 * @brief specific toggle for HTTP server DEFLATE compression support.
 */
#ifndef mla_http_server_use_deflate_compression
#define mla_http_server_use_deflate_compression mla_http_use_deflate_compression
#endif

/**
 * @brief specific toggle for HTTP client DEFLATE compression support.
 */
#ifndef mla_http_client_use_deflate_compression
#define mla_http_client_use_deflate_compression mla_http_use_deflate_compression
#endif

/**
 * @brief specific toggle for HTTP client GZIP compression support.
 */
#ifndef mla_http_client_use_gzip_compression
#define mla_http_client_use_gzip_compression mla_http_use_deflate_compression
#endif

/////////////////////////////////////////////////
// Task and Lock Configuration
/////////////////////////////////////////////////

/**
 * @brief Default maximum wait time in milliseconds for acquiring a mutex lock.
 */
#ifndef mla_mutex_default_wait_timeout
#define mla_mutex_default_wait_timeout 1000
#endif

/**
 * @brief Default maximum wait time in milliseconds for acquiring a read/write lock.
 */
#ifndef mla_rw_lock_default_timeout
#define mla_rw_lock_default_timeout 1000
#endif

/////////////////////////////////////////////////
// Memory Hook Configuration
/////////////////////////////////////////////////

/**
 * @brief Maximum number of simultaneous memory hooks allowed in the framework.
 */
#ifndef CONST_MLA_MAX_MEMORY_HOOKS
#define CONST_MLA_MAX_MEMORY_HOOKS 10
#endif

/////////////////////////////////////////////////
// RPC Configuration
/////////////////////////////////////////////////

/**
 * @brief Size of the small buffer used for RPC stream operations.
 */
#ifndef mla_rpc_stream_small_buffer_size
#define mla_rpc_stream_small_buffer_size 1024 // 1 KB
#endif

#endif // COREOS_MLA_GLOBAL_CONFIG_H

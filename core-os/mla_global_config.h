#ifndef MLA_C_MLA_GLOBAL_CONFIG_H
#define MLA_C_MLA_GLOBAL_CONFIG_H

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
#ifndef mla_global_config_rpc_log_cache_size
#define mla_global_config_rpc_log_cache_size 128
#endif

/**
 * @brief Flag to enable native logging output.
 * If set to 1, log messages are printed directly to the system output using mla_print.
 */
#ifndef mla_global_feature_flag_logging_use_native
#define mla_global_feature_flag_logging_use_native 0
#endif

/**
 * @brief Default log level for the console logger.
 * Messages with a priority lower than this level will be filtered out.
 * 0: VERBOSE, 1: DEBUG, 2: INFO, 3: WARNING, 4: ERROR
 */
#ifndef mla_global_config_logger_console_level
#define mla_global_config_logger_console_level ((mla_log_level)2) // corresponds to MLA_LOG_LEVEL_INFO
#endif

/////////////////////////////////////////////////
// String Configuration
/////////////////////////////////////////////////

/**
 * @brief Maximum length for Small String Optimization (SSO).
 * Strings up to this length (in bytes) are stored within the string structure itself,
 * avoiding heap allocation.
 */
#ifndef mla_global_config_string_sso_max_length
#define mla_global_config_string_sso_max_length 14
#endif

/////////////////////////////////////////////////
// Data Structure Defaults
/////////////////////////////////////////////////

/**
 * @brief Default initial capacity for mla_array_list_t.
 */
#ifndef mla_global_config_array_list_default_capacity
#define mla_global_config_array_list_default_capacity 8
#endif

/**
 * @brief Default initial size for items list within a hash map bucket.
 */
#ifndef mla_global_config_hash_map_item_default_size
#define mla_global_config_hash_map_item_default_size 2
#endif

/**
 * @brief Default number of buckets for a new hash map.
 */
#ifndef mla_global_config_hash_map_default_bucket_size
#define mla_global_config_hash_map_default_bucket_size 8
#endif

/**
 * @brief Default load factor for hash map rebalancing.
 * When the ratio of items to buckets exceeds this value, the map is resized.
 */
#ifndef mla_global_config_hash_map_default_load_factor
#define mla_global_config_hash_map_default_load_factor 0.75
#endif

/////////////////////////////////////////////////
// Stream Configuration
/////////////////////////////////////////////////

/**
 * @brief Buffer size for high-speed or non-blocking stream reads.
 * Used to balance memory usage and I/O efficiency.
 */
#ifndef mla_global_config_stream_fast_read_buffer_size
#define mla_global_config_stream_fast_read_buffer_size 128
#endif

/**
 * @brief Minimum data size required to trigger DEFLATE compression.
 * Smaller data may result in a larger compressed output or inefficient processing.
 */
#ifndef mla_global_config_stream_output_deflate_min_compression_data_size
#define mla_global_config_stream_output_deflate_min_compression_data_size 1024
#endif

/////////////////////////////////////////////////
// HTTP Configuration
/////////////////////////////////////////////////

/**
 * @brief Default timeout in milliseconds for HTTP client and server operations.
 */
#ifndef mla_global_config_default_http_timeout_ms
#define mla_global_config_default_http_timeout_ms 30000
#endif

/**
 * @brief Global toggle for DEFLATE compression support in HTTP modules.
 */
#ifndef mla_global_feature_flag_http_use_deflate_compression
#define mla_global_feature_flag_http_use_deflate_compression 1
#endif

/**
 * @brief Maximum permitted size for HTTP headers in bytes.
 */
#ifndef mla_global_config_http_max_header_size
#define mla_global_config_http_max_header_size 8192 // 8 KB
#endif

/**
 * @brief specific toggle for HTTP server DEFLATE compression support.
 */
#ifndef mla_global_feature_flag_http_server_use_deflate_compression
#define mla_global_feature_flag_http_server_use_deflate_compression mla_global_feature_flag_http_use_deflate_compression
#endif

/**
 * @brief specific toggle for HTTP client DEFLATE compression support.
 */
#ifndef mla_global_feature_flag_http_client_use_deflate_compression
#define mla_global_feature_flag_http_client_use_deflate_compression mla_global_feature_flag_http_use_deflate_compression
#endif

/**
 * @brief specific toggle for HTTP client GZIP compression support.
 */
#ifndef mla_global_feature_flag_http_client_use_gzip_compression
#define mla_global_feature_flag_http_client_use_gzip_compression mla_global_feature_flag_http_use_deflate_compression
#endif

/////////////////////////////////////////////////
// Task and Lock Configuration
/////////////////////////////////////////////////

/**
 * @brief Default maximum wait time in milliseconds for acquiring a mutex lock.
 */
#ifndef mla_global_config_mutex_default_wait_timeout
#define mla_global_config_mutex_default_wait_timeout 1000
#endif

/**
 * @brief Default maximum wait time in milliseconds for acquiring a read/write lock.
 */
#ifndef mla_global_config_rw_lock_default_timeout
#define mla_global_config_rw_lock_default_timeout 1000
#endif

/////////////////////////////////////////////////
// Memory Hook Configuration
/////////////////////////////////////////////////

/**
 * @brief Maximum number of simultaneous memory hooks allowed in the framework.
 */
#ifndef mla_global_config_max_memory_hooks
#define mla_global_config_max_memory_hooks 10
#endif

/////////////////////////////////////////////////
// RPC Configuration
/////////////////////////////////////////////////

/**
 * @brief Size of the small buffer used for RPC stream operations.
 */
#ifndef mla_global_config_rpc_stream_small_buffer_size
#define mla_global_config_rpc_stream_small_buffer_size 1024 // 1 KB
#endif

#endif // COREOS_MLA_GLOBAL_CONFIG_H

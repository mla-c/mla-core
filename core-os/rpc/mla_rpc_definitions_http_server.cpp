//
// Created by copilot on 05/05/26.
//
// HTTP GET /rpc/definitions.ts
// Returns a TypeScript file with interfaces and service classes derived from
// all registered RPC procedures and their reflection metadata.
//

#include "mla_rpc_definitions_http_server.h"
#include "mla_rpc.h"
#include "../reflection/mla_reflection.h"
#include "../system/mla_stream.h"
#include "../system/mla_string_concat.h"
#include "../log/mla_logging.h"

// ---------------------------------------------------------------------------
// Name-conversion helpers
// ---------------------------------------------------------------------------

// Capitalize the first ASCII letter of a string segment (e.g. "logger" -> "Logger").
static mla_string_t __mla_rpc_ts_capitalise_first(const mla_string_t& segment) {

    mla_size_t len = mla_string_length(segment);

    if (len == 0) {
        return mla_string_empty();
    }

    const mla_char_t* src = mla_string_data(segment);

    // Use a fixed-size stack buffer; struct name segments are short.
    mla_char_t buf[256];
    mla_size_t copy_len = len < 255 ? len : 255;

    for (mla_size_t i = 0; i < copy_len; i++) {
        buf[i] = src[i];
    }

    if (buf[0] >= 'a' && buf[0] <= 'z') {
        buf[0] = (mla_char_t)(buf[0] - 'a' + 'A');
    }

    return mla_string_copy(buf, copy_len);
}

// Convert a C++ struct name like "mla_logger_rpc_log_level_t" to a TypeScript
// interface name like "LoggerRpcLogLevel":
//   1. Strip leading "mla_" prefix.
//   2. Strip trailing "_t" suffix.
//   3. Split on '_', capitalise each segment, join.
static mla_string_t __mla_rpc_ts_struct_to_interface_name(const mla_string_t& struct_name) {

    mla_string_t name = struct_name;

    if (mla_string_starts_with(name, mla_string_const("mla_"))) {
        name = mla_string_substr(name, 4);
    }

    if (mla_string_ends_with(name, mla_string_const("_t"))) {
        mla_size_t len = mla_string_length(name);
        name = mla_string_substr(name, 0, len - 2);
    }

    auto parts = mla_string_split(name, mla_string_const("_"));

    mla_string_t result = mla_string_empty();

    for (mla_size_t i = 0; i < mla_array_list_size(parts); i++) {
        const mla_string_t& part = mla_array_list_get_unsafe(parts, i);
        if (mla_string_is_empty(part)) {
            continue;
        }
        mla_string_t cap = __mla_rpc_ts_capitalise_first(part);
        result = mla_string_concat(result, cap);
    }

    return result;
}

// Map a reflection primitive/list type to a TypeScript type string.
// For STRUCT fields the caller must supply the resolved interface name.
static mla_string_t __mla_rpc_ts_primitive_type(mla_reflection_type_t type) {

    switch (type) {
        case MLA_REFLECTION_TYPE_BOOL:
            return mla_string_const("boolean");
        case MLA_REFLECTION_TYPE_STRING:
        case MLA_REFLECTION_TYPE_BYTES:
            return mla_string_const("string");
        default:
            return mla_string_const("number");
    }
}

// Return the TypeScript type expression for a struct field.
static mla_string_t __mla_rpc_ts_field_type(const mla_reflection_struct_field_t& field) {

    if (field.type == MLA_REFLECTION_TYPE_STRUCT) {
        if (field.struct_provider != nullptr) {
            mla_reflection_struct_metadata_t meta = field.struct_provider();
            return __mla_rpc_ts_struct_to_interface_name(meta.name);
        }
        return mla_string_const("object");
    }

    if (field.type == MLA_REFLECTION_TYPE_LIST) {
        mla_string_t element_type = mla_string_empty();
        if (field.element_type == MLA_REFLECTION_TYPE_STRUCT) {
            if (field.struct_provider != nullptr) {
                mla_reflection_struct_metadata_t meta = field.struct_provider();
                element_type = __mla_rpc_ts_struct_to_interface_name(meta.name);
            } else {
                element_type = mla_string_const("object");
            }
        } else {
            element_type = __mla_rpc_ts_primitive_type(field.element_type);
        }
        return mla_string_concat(element_type, mla_string_const("[]"));
    }

    return __mla_rpc_ts_primitive_type(field.type);
}

// ---------------------------------------------------------------------------
// TypeScript writers
// ---------------------------------------------------------------------------

static void __mla_rpc_ts_write(mla_stream_output_t& out, const mla_string_t& text) {
    mla_stream_output_write_string(out, text);
}

static void __mla_rpc_ts_writeln(mla_stream_output_t& out, const mla_string_t& text) {
    mla_stream_output_write_string(out, text);
    mla_stream_output_write_string(out, mla_string_const("\n"));
}

// Write a TypeScript interface for one registered struct.
static void __mla_rpc_ts_write_interface(mla_stream_output_t& out, const mla_reflection_struct_metadata_t& metadata) {

    mla_string_t interface_name = __mla_rpc_ts_struct_to_interface_name(metadata.name);

    __mla_rpc_ts_write(out, mla_string_const("export interface "));
    __mla_rpc_ts_writeln(out, interface_name);
    __mla_rpc_ts_writeln(out, mla_string_const("{"));

    for (mla_size_t i = 0; i < mla_array_list_size(metadata.fields); i++) {
        const mla_reflection_struct_field_t& field = mla_array_list_get_unsafe(metadata.fields, i);
        mla_string_t ts_type = __mla_rpc_ts_field_type(field);
        __mla_rpc_ts_write(out, mla_string_const("    "));
        __mla_rpc_ts_write(out, field.name);
        __mla_rpc_ts_write(out, mla_string_const(": "));
        __mla_rpc_ts_write(out, ts_type);
        __mla_rpc_ts_writeln(out, mla_string_const(";"));
    }

    __mla_rpc_ts_writeln(out, mla_string_const("}"));
    __mla_rpc_ts_writeln(out, mla_string_empty());
}

// Extract the prefix (part before the first '/') from a procedure name.
// If there is no '/', the full name is returned.
static mla_string_t __mla_rpc_ts_procedure_prefix(const mla_string_t& procedure_name) {

    mla_int32_t idx = mla_string_index_of(procedure_name, mla_string_const("/"));

    if (idx < 0) {
        return procedure_name;
    }

    return mla_string_substr(procedure_name, 0, (mla_size_t)idx);
}

// Extract the method name (part after the first '/') from a procedure name.
// If there is no '/', the full name is returned.
static mla_string_t __mla_rpc_ts_procedure_method(const mla_string_t& procedure_name) {

    mla_int32_t idx = mla_string_index_of(procedure_name, mla_string_const("/"));

    if (idx < 0) {
        return procedure_name;
    }

    return mla_string_substr(procedure_name, (mla_size_t)idx + 1);
}

// Build the service class name from a prefix, e.g. "log" -> "LogService".
static mla_string_t __mla_rpc_ts_class_name(const mla_string_t& prefix) {
    mla_string_t cap = __mla_rpc_ts_capitalise_first(prefix);
    return mla_string_concat(cap, mla_string_const("Service"));
}

// Write a single service method for one procedure.
static void __mla_rpc_ts_write_method(mla_stream_output_t& out, const mla_rpc_procedure_unsafe_t& proc) {

    mla_string_t method_name = __mla_rpc_ts_procedure_method(proc.procedureName);

    mla_bool_t has_input  = proc.inputDefinition.data_size > 0;
    mla_bool_t has_output = proc.outputDefinition.data_size > 0;

    mla_string_t input_type  = mla_string_empty();
    mla_string_t output_type = mla_string_empty();

    if (has_input && !mla_string_is_empty(proc.inputStructName)) {
        input_type = __mla_rpc_ts_struct_to_interface_name(proc.inputStructName);
    }

    if (has_output && !mla_string_is_empty(proc.outputStructName)) {
        output_type = __mla_rpc_ts_struct_to_interface_name(proc.outputStructName);
    }

    // Signature: public static methodName(request?: InputType): Promise<OutputType | void>
    __mla_rpc_ts_write(out, mla_string_const("    public static "));
    __mla_rpc_ts_write(out, method_name);
    __mla_rpc_ts_write(out, mla_string_const("("));

    if (has_input && !mla_string_is_empty(input_type)) {
        __mla_rpc_ts_write(out, mla_string_const("request: "));
        __mla_rpc_ts_write(out, input_type);
    }

    __mla_rpc_ts_write(out, mla_string_const("): "));

    if (has_output && !mla_string_is_empty(output_type)) {
        __mla_rpc_ts_write(out, mla_string_const("Promise<"));
        __mla_rpc_ts_write(out, output_type);
        __mla_rpc_ts_writeln(out, mla_string_const("> {"));
    } else {
        __mla_rpc_ts_writeln(out, mla_string_const("Promise<void> {"));
    }

    // Body: return ApiService.call(...)
    __mla_rpc_ts_write(out, mla_string_const("        return ApiService.call"));

    if (has_output && !mla_string_is_empty(output_type)) {
        __mla_rpc_ts_write(out, mla_string_const("<"));
        __mla_rpc_ts_write(out, output_type);
        __mla_rpc_ts_write(out, mla_string_const(">"));
    }

    __mla_rpc_ts_write(out, mla_string_const("(\""));
    __mla_rpc_ts_write(out, proc.procedureName);
    __mla_rpc_ts_write(out, mla_string_const("\""));

    if (has_input && !mla_string_is_empty(input_type)) {
        __mla_rpc_ts_write(out, mla_string_const(", request"));
    }

    __mla_rpc_ts_writeln(out, mla_string_const(");"));
    __mla_rpc_ts_writeln(out, mla_string_const("    }"));
    __mla_rpc_ts_writeln(out, mla_string_empty());
}

// Write one service class for a given prefix.
static void __mla_rpc_ts_write_service_class(
    mla_stream_output_t& out,
    const mla_string_t& prefix,
    const mla_array_list_t<mla_rpc_procedure_unsafe_t, mla_rpc_procedure_unsafe_initializer>& all_procedures
) {
    mla_string_t class_name = __mla_rpc_ts_class_name(prefix);

    __mla_rpc_ts_write(out, mla_string_const("export class "));
    __mla_rpc_ts_writeln(out, class_name);
    __mla_rpc_ts_writeln(out, mla_string_const("{"));
    __mla_rpc_ts_writeln(out, mla_string_empty());

    for (mla_size_t i = 0; i < mla_array_list_size(all_procedures); i++) {
        const mla_rpc_procedure_unsafe_t& proc = mla_array_list_get_unsafe(all_procedures, i);
        mla_string_t proc_prefix = __mla_rpc_ts_procedure_prefix(proc.procedureName);
        if (mla_string_equals(proc_prefix, prefix)) {
            __mla_rpc_ts_write_method(out, proc);
        }
    }

    __mla_rpc_ts_writeln(out, mla_string_const("}"));
    __mla_rpc_ts_writeln(out, mla_string_empty());
}

// ---------------------------------------------------------------------------
// Main TypeScript generation
// ---------------------------------------------------------------------------

static void __mla_rpc_ts_write_definitions(mla_stream_output_t& out) {

    // Header
    __mla_rpc_ts_writeln(out, mla_string_const("// Auto-generated by rpc/definitions.ts endpoint."));
    __mla_rpc_ts_writeln(out, mla_string_const("// Do not edit manually."));
    __mla_rpc_ts_writeln(out, mla_string_empty());
    __mla_rpc_ts_writeln(out, mla_string_const("import {ApiService} from \"./ApiService\";"));
    __mla_rpc_ts_writeln(out, mla_string_empty());

    // Interfaces - iterate all registered structs
    __mla_rpc_ts_writeln(out, mla_string_const("// -----------------------------------------------------------------------"));
    __mla_rpc_ts_writeln(out, mla_string_const("// Interfaces"));
    __mla_rpc_ts_writeln(out, mla_string_const("// -----------------------------------------------------------------------"));
    __mla_rpc_ts_writeln(out, mla_string_empty());

    auto struct_names = mla_reflection_list_struct_names();

    for (mla_size_t i = 0; i < mla_array_list_size(struct_names); i++) {
        const mla_string_t& name = mla_array_list_get_unsafe(struct_names, i);
        mla_reflection_struct_metadata_t metadata = mla_reflection_struct_metadata_invalid();
        if (!mla_reflection_get_struct_metadata(name, metadata)) {
            continue;
        }
        __mla_rpc_ts_write_interface(out, metadata);
    }

    // Service classes - group procedures by prefix
    __mla_rpc_ts_writeln(out, mla_string_const("// -----------------------------------------------------------------------"));
    __mla_rpc_ts_writeln(out, mla_string_const("// Services"));
    __mla_rpc_ts_writeln(out, mla_string_const("// -----------------------------------------------------------------------"));
    __mla_rpc_ts_writeln(out, mla_string_empty());

    auto procedures = mla_rpc_list_procedures();

    // Collect unique prefixes
    auto prefixes = mla_array_list<mla_string_t, mla_string_initializer>(8);

    for (mla_size_t i = 0; i < mla_array_list_size(procedures); i++) {
        const mla_rpc_procedure_unsafe_t& proc = mla_array_list_get_unsafe(procedures, i);
        mla_string_t prefix = __mla_rpc_ts_procedure_prefix(proc.procedureName);

        mla_bool_t found = false;
        for (mla_size_t j = 0; j < mla_array_list_size(prefixes); j++) {
            if (mla_string_equals(mla_array_list_get_unsafe(prefixes, j), prefix)) {
                found = true;
                break;
            }
        }

        if (!found) {
            mla_array_list_add(prefixes, prefix);
        }
    }

    // Write one service class per prefix
    for (mla_size_t i = 0; i < mla_array_list_size(prefixes); i++) {
        __mla_rpc_ts_write_service_class(out, mla_array_list_get_unsafe(prefixes, i), procedures);
    }
}

// ---------------------------------------------------------------------------
// HTTP handler
// ---------------------------------------------------------------------------

static mla_bool_t __mla_rpc_definitions_http_server_handler(
    mla_http_server_t& http_server,
    const mla_http_request_t& request,
    mla_http_response_t& response
) {
    (void)http_server;
    (void)request;

    // Generate all TypeScript content into a growable memory stream.
    mla_memory_stream_t content_stream = mla_memory_stream(4096, true);

    __mla_rpc_ts_write_definitions(content_stream.output);

    mla_size_t content_size = mla_memory_stream_get_size(content_stream);
    mla_memory_stream_set_position(content_stream, 0);

    response.statusCode  = mla_http_status_ok;
    response.statusMessage = mla_string_const("OK");
    mla_http_headers_add(response.headers, mla_string_const("Content-Type"),                mla_string_const("application/typescript"));
    mla_http_headers_add(response.headers, mla_string_const("Access-Control-Allow-Origin"), mla_string_const("*"));
    mla_http_headers_add(response.headers, mla_string_const("Content-Length"),              mla_string_from_size(content_size));

    response.content = content_stream.input;

    return true;
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

mla_bool_t mla_rpc_definitions_http_server_initialize(mla_http_server_t& server) {

    mla_http_server_handler_item_t handler = mla_http_server_handler_starts_with(
        mla_http_method_get,
        mla_string_const("/rpc/definitions"),
        __mla_rpc_definitions_http_server_handler
    );

    return mla_http_server_register_handler(server, handler);
}

//
// Created by chris on 6/30/2026.
//

#ifndef MLA_UI_HTML_TEMPLATE_COMPILE_TEST_H
#define MLA_UI_HTML_TEMPLATE_COMPILE_TEST_H

#include "../../lib/base-lib/core/ui/web/mla_ui_html_template_compile.h"
#include "../../lib/base-lib/test-support/mla_test_executor.h"

mla_user_data_id_init(mla_ui_html_template_compile_test_capture_state_user_data_name)

struct mla_ui_html_template_compile_test_capture_state_t {
    mla_stream_output_t* output;
};

static mla_size_t g_mla_ui_html_template_compile_test_chunk_size = 1;

inline mla_size_t mla_ui_html_template_compile_test_throttled_read(mla_stream_input_t& wrapper, mla_stream_input_t& input, mla_size_t offset, mla_size_t length, const mla_byte_t* buffer) {
    (void)wrapper;

    if (buffer == nullptr || length == 0) {
        return 0;
    }

    mla_size_t to_read = length;
    if (g_mla_ui_html_template_compile_test_chunk_size < to_read) {
        to_read = g_mla_ui_html_template_compile_test_chunk_size;
    }

    return input.read(input, offset, to_read, const_cast<mla_byte_t*>(buffer));
}

inline mla_ui_html_template_compile_test_capture_state_t* mla_ui_html_template_compile_test_get_capture_state(mla_ui_html_template_compile_writer_t& writer) {
    return mla_user_data_get_pointer_data<mla_ui_html_template_compile_test_capture_state_t>(writer.user_data, mla_ui_html_template_compile_test_capture_state_user_data_name);
}

inline mla_bool_t mla_ui_html_template_compile_test_emit_html(mla_ui_html_template_compile_writer_t& writer, const mla_string_t& html_block) {
    mla_ui_html_template_compile_test_capture_state_t* capture_state = mla_ui_html_template_compile_test_get_capture_state(writer);

    if (capture_state == nullptr || capture_state->output == nullptr) {
        return false;
    }

    return mla_stream_output_write_string(*capture_state->output, mla_string_const("[H]"))
        && mla_stream_output_write_string(*capture_state->output, html_block)
        && mla_stream_output_write_string(*capture_state->output, mla_string_const("\n"));
}

inline mla_bool_t mla_ui_html_template_compile_test_emit_code(mla_ui_html_template_compile_writer_t& writer, const mla_string_t& code_block) {
    mla_ui_html_template_compile_test_capture_state_t* capture_state = mla_ui_html_template_compile_test_get_capture_state(writer);

    if (capture_state == nullptr || capture_state->output == nullptr) {
        return false;
    }

    return mla_stream_output_write_string(*capture_state->output, mla_string_const("[C]"))
        && mla_stream_output_write_string(*capture_state->output, code_block)
        && mla_stream_output_write_string(*capture_state->output, mla_string_const("\n"));
}

inline void UiHtmlTemplateCompilePlainHtmlTest() {
    mla_string_t source = mla_string_const("1 < 2 and 3 > 2");
    mla_stream_input_t input = mla_stream_input_from_string(source);
    mla_memory_stream_t output = mla_memory_stream_empty();

    mla_ui_html_template_compile_writer_t writer = mla_ui_html_template_compile_writer_html_only(output.output);

    assert_true(mla_ui_html_template_compile(input, writer), "Compile should succeed for plain HTML");

    assert_true(mla_memory_stream_set_position(output, 0), "Should reset output stream before reading");
    mla_string_t actual = mla_string_from_stream(output.input, mla_memory_stream_get_size(output) + 1);

    assert_true(mla_string_equals(actual, source), "Plain HTML should be written unchanged");
}

inline void UiHtmlTemplateCompileMixedBlocksChunkedTest() {
    mla_string_t source = mla_string_const("A<%B%>C");
    mla_stream_input_t base_input = mla_stream_input_from_string(source);
    mla_stream_input_t chunked_input = mla_stream_input_interceptor_wrapper(base_input, mla_ui_html_template_compile_test_throttled_read, nullptr);

    mla_memory_stream_t output = mla_memory_stream_empty();
    mla_ui_html_template_compile_test_capture_state_t capture_state = { &output.output };

    mla_ui_html_template_compile_writer_t writer = {
        mla_user_data_empty(),
        mla_ui_html_template_compile_test_emit_html,
        mla_ui_html_template_compile_test_emit_code
    };

    mla_user_data_t user_data = mla_user_data_empty();
    mla_pointer_t capture_state_ptr = mla_platform_pointer_to_managed_pointer(&capture_state);
    assert_true(mla_user_data_set_pointer(user_data, mla_ui_html_template_compile_test_capture_state_user_data_name, capture_state_ptr),
                "Should attach capture state to writer user data");
    writer.user_data = user_data;

    g_mla_ui_html_template_compile_test_chunk_size = 1;

    assert_true(mla_ui_html_template_compile(chunked_input, writer), "Compile should succeed for mixed template blocks");

    assert_true(mla_memory_stream_set_position(output, 0), "Should reset output stream before reading");
    mla_string_t actual = mla_string_from_stream(output.input, mla_memory_stream_get_size(output) + 1);
    mla_string_t expected = mla_string_const("[H]A\n[C]B\n[H]C\n");

    assert_true(mla_string_equals(actual, expected), "Compiler should split HTML and code blocks and remove template markers");
}

inline void UiHtmlTemplateCompileComplexMixedBlocksTest() {
    mla_string_t source = mla_string_const("Start < 1 and <%alpha%> mid <%beta%> tail < end");
    mla_stream_input_t base_input = mla_stream_input_from_string(source);
    mla_stream_input_t chunked_input = mla_stream_input_interceptor_wrapper(base_input, mla_ui_html_template_compile_test_throttled_read, nullptr);

    mla_memory_stream_t output = mla_memory_stream_empty();
    mla_ui_html_template_compile_test_capture_state_t capture_state = { &output.output };

    mla_ui_html_template_compile_writer_t writer = {
        mla_user_data_empty(),
        mla_ui_html_template_compile_test_emit_html,
        mla_ui_html_template_compile_test_emit_code
    };

    mla_user_data_t user_data = mla_user_data_empty();
    mla_pointer_t capture_state_ptr = mla_platform_pointer_to_managed_pointer(&capture_state);
    assert_true(mla_user_data_set_pointer(user_data, mla_ui_html_template_compile_test_capture_state_user_data_name, capture_state_ptr),
                "Should attach capture state to writer user data");
    writer.user_data = user_data;

    g_mla_ui_html_template_compile_test_chunk_size = 2;

    assert_true(mla_ui_html_template_compile(chunked_input, writer), "Compile should succeed for complex mixed template blocks");

    assert_true(mla_memory_stream_set_position(output, 0), "Should reset output stream before reading");
    mla_string_t actual = mla_string_from_stream(output.input, mla_memory_stream_get_size(output) + 1);
    mla_string_t expected = mla_string_const("[H]Start < 1 and \n[C]alpha\n[H] mid \n[C]beta\n[H] tail < end\n");

    assert_true(mla_string_equals(actual, expected), "Compiler should preserve literal angle brackets and handle multiple template transitions");
}

inline void UiHtmlTemplateCompileInvalidInputUnclosedCodeBlockTest() {
    mla_string_t source = mla_string_const("prefix <%unclosed code block");
    mla_stream_input_t base_input = mla_stream_input_from_string(source);
    mla_stream_input_t chunked_input = mla_stream_input_interceptor_wrapper(base_input, mla_ui_html_template_compile_test_throttled_read, nullptr);

    mla_memory_stream_t output = mla_memory_stream_empty();
    mla_ui_html_template_compile_test_capture_state_t capture_state = { &output.output };

    mla_ui_html_template_compile_writer_t writer = {
        mla_user_data_empty(),
        mla_ui_html_template_compile_test_emit_html,
        mla_ui_html_template_compile_test_emit_code
    };

    mla_user_data_t user_data = mla_user_data_empty();
    mla_pointer_t capture_state_ptr = mla_platform_pointer_to_managed_pointer(&capture_state);
    assert_true(mla_user_data_set_pointer(user_data, mla_ui_html_template_compile_test_capture_state_user_data_name, capture_state_ptr),
                "Should attach capture state to writer user data");
    writer.user_data = user_data;

    g_mla_ui_html_template_compile_test_chunk_size = 3;

    assert_true(mla_ui_html_template_compile(chunked_input, writer), "Compiler should not fail on an unclosed code block");

    assert_true(mla_memory_stream_set_position(output, 0), "Should reset output stream before reading");
    mla_string_t actual = mla_string_from_stream(output.input, mla_memory_stream_get_size(output) + 1);
    mla_string_t expected = mla_string_const("[H]prefix \n[C]unclosed code block\n");

    assert_true(mla_string_equals(actual, expected), "Compiler should treat trailing unclosed code block content as code");
}

inline void RegisterUiHtmlTemplateCompileTests(mla_test_executor_t& p_TestExecutor) {

    mla_test_t test = mla_test("PlainHtml", test_category, UiHtmlTemplateCompilePlainHtmlTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("MixedBlocksChunked", test_category, UiHtmlTemplateCompileMixedBlocksChunkedTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ComplexMixedBlocks", test_category, UiHtmlTemplateCompileComplexMixedBlocksTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("InvalidInputUnclosedCodeBlock", test_category, UiHtmlTemplateCompileInvalidInputUnclosedCodeBlockTest);
    mla_test_executor_register_test(p_TestExecutor, test);
}

#endif //MLA_UI_HTML_TEMPLATE_COMPILE_TEST_H

//
// Created by chris on 6/30/2026.
//

#ifndef MLA_UI_HTML_TEMPLATE_COMPILE_H
#define MLA_UI_HTML_TEMPLATE_COMPILE_H
#include "../../mla_data_types.h"
#include "../../system/mla_stream.h"

struct mla_ui_html_template_compile_writer_t {
    mla_user_data_t user_data;
    mla_bool_t (*emit_html)(mla_ui_html_template_compile_writer_t& writer, const mla_string_t& html_block);
    mla_bool_t (*emit_code)(mla_ui_html_template_compile_writer_t& writer, const mla_string_t& code_block);
};

mla_bool_t mla_ui_html_template_compile(mla_stream_input_t& input_stream, mla_ui_html_template_compile_writer_t& writer);

mla_ui_html_template_compile_writer_t mla_ui_html_template_compile_writer_noop();
mla_ui_html_template_compile_writer_t mla_ui_html_template_compile_writer_html_only(mla_stream_output_t& output_stream);



#endif //MLA_UI_HTML_TEMPLATE_COMPILE_H

//
// Created by chris on 1/15/2026.
//

#ifndef COREOS_MLA_UI_CONTROL_DATA_TYPES_H
#define COREOS_MLA_UI_CONTROL_DATA_TYPES_H

#include "../../system/mla_string.h"
#include "../../system/mla_stream.h"
#include "../../system/mla_array_list.h"
#include "../surfaces/mla_ui_surface_draw.h"

struct mla_ui_control_t;

typedef mla_ui_surface_draw_size_t (mla_ui_control_context_calcTextSize_t)(const mla_string_t &fontFamily, mla_double_t fontSize, const mla_string_t &text);

struct mla_ui_control_context_t {
    mla_size_t offsetX;
    mla_size_t offsetY;
    mla_size_t width;
    mla_size_t height;
    mla_size_t timeSinceLastFrameMs;
    mla_ui_surface_input_states_t input_states;
    mla_ui_control_context_calcTextSize_t *calcTextSize;
};

mla_ui_control_context_t mla_ui_control_context(mla_size_t width, mla_size_t height, const mla_ui_surface_input_states_t& input_states, mla_ui_control_context_calcTextSize_t *calcTextSize);
mla_ui_control_context_t mla_ui_control_create_context_for_children(const mla_ui_control_context_t &parentContext, const mla_ui_control_t &control);

struct mla_ui_control_value_t {
    mla_string_t name;
    mla_string_t stringValue;

    union {
        mla_int64_t int64Value;
        mla_uint64_t uint64Value;
        mla_double_t doubleValue;
        mla_bool_t boolValue;
    };
};

mla_ui_control_value_t mla_ui_control_value_empty();

struct mla_ui_control_value_initializer_t {
    static mla_ui_control_value_t init() {
        return mla_ui_control_value_empty();
    }
};

typedef mla_bool_t (*mla_ui_control_render_to_draw_commands_t)(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands);

struct mla_ui_control_initializer_t;

struct mla_ui_control_layout_t {
    mla_size_t x;
    mla_size_t y;
    mla_size_t width;
    mla_size_t height;
};

struct mla_ui_control_t {
    mla_string_t id;
    mla_ui_control_layout_t layout;
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> children;
    mla_ui_control_render_to_draw_commands_t renderToDrawCommands;
    mla_array_list_t<mla_ui_control_value_t, mla_ui_control_value_initializer_t> values;
};

mla_ui_control_t mla_ui_control();
mla_ui_control_t mla_ui_control_empty();

struct mla_ui_control_initializer_t {
    static mla_ui_control_t init() {
        return mla_ui_control_empty();
    }
};

mla_uint8_t mla_ui_control_get_value_as_uint8(const mla_ui_control_t &control, const mla_string_t &name, mla_uint8_t defaultValue = 0);
mla_int8_t mla_ui_control_get_value_as_int8(const mla_ui_control_t &control, const mla_string_t &name, mla_int8_t defaultValue = 0);
mla_uint16_t mla_ui_control_get_value_as_uint16(const mla_ui_control_t &control, const mla_string_t &name, mla_uint16_t defaultValue = 0);
mla_int16_t mla_ui_control_get_value_as_int16(const mla_ui_control_t &control, const mla_string_t &name, mla_int16_t defaultValue = 0);
mla_uint32_t mla_ui_control_get_value_as_uint32(const mla_ui_control_t &control, const mla_string_t &name, mla_uint32_t defaultValue = 0);
mla_int32_t mla_ui_control_get_value_as_int32(const mla_ui_control_t &control, const mla_string_t &name, mla_int32_t defaultValue = 0);
mla_uint64_t mla_ui_control_get_value_as_uint64(const mla_ui_control_t &control, const mla_string_t &name, mla_uint64_t defaultValue = 0);
mla_int64_t mla_ui_control_get_value_as_int64(const mla_ui_control_t &control, const mla_string_t &name, mla_int64_t defaultValue = 0);
mla_float_t mla_ui_control_get_value_as_float(const mla_ui_control_t &control, const mla_string_t &name, mla_float_t defaultValue = 0.0f);
mla_double_t mla_ui_control_get_value_as_double(const mla_ui_control_t &control, const mla_string_t &name, mla_double_t defaultValue = 0.0);
mla_string_t mla_ui_control_get_value_as_string(const mla_ui_control_t &control, const mla_string_t &name, const mla_string_t &defaultValue = mla_string_empty());
mla_bool_t mla_ui_control_get_value_as_bool(const mla_ui_control_t &control, const mla_string_t &name, mla_bool_t defaultValue = false);

mla_bool_t mla_ui_control_set_value_as_uint8(mla_ui_control_t &control, const mla_string_t &name, mla_uint8_t value);
mla_bool_t mla_ui_control_set_value_as_int8(mla_ui_control_t &control, const mla_string_t &name, mla_int8_t value);
mla_bool_t mla_ui_control_set_value_as_uint16(mla_ui_control_t &control, const mla_string_t &name, mla_uint16_t value);
mla_bool_t mla_ui_control_set_value_as_int16(mla_ui_control_t &control, const mla_string_t &name, mla_int16_t value);
mla_bool_t mla_ui_control_set_value_as_uint32(mla_ui_control_t &control, const mla_string_t &name, mla_uint32_t value);
mla_bool_t mla_ui_control_set_value_as_int32(mla_ui_control_t &control, const mla_string_t &name, mla_int32_t value);
mla_bool_t mla_ui_control_set_value_as_uint64(mla_ui_control_t &control, const mla_string_t &name, mla_uint64_t value);
mla_bool_t mla_ui_control_set_value_as_int64(mla_ui_control_t &control, const mla_string_t &name, mla_int64_t value);
mla_bool_t mla_ui_control_set_value_as_float(mla_ui_control_t &control, const mla_string_t &name, mla_float_t value);
mla_bool_t mla_ui_control_set_value_as_double(mla_ui_control_t &control, const mla_string_t &name, mla_double_t value);
mla_bool_t mla_ui_control_set_value_as_string(mla_ui_control_t &control, const mla_string_t &name, const mla_string_t &value);
mla_bool_t mla_ui_control_set_value_as_bool(mla_ui_control_t &control, const mla_string_t &name, mla_bool_t value);

mla_bool_t mla_ui_control_add_child(mla_ui_control_t &parent, const mla_ui_control_t &child);
mla_bool_t mla_ui_control_render_to_draw_commands(const mla_ui_control_context_t &context, const mla_ui_control_t &control, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands);

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_t &control);
mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_layout_t &layout);

#define mla_ui_control_render_children_draw_commands(context, control, output) \
mla_size_t childrenCount = mla_array_list_size((control).children); \
if (childrenCount > 0) { \
    mla_ui_control_context_t __childContext = mla_ui_control_create_context_for_children(context, control); \
    for (mla_size_t __i = 0; __i < childrenCount; __i++) { \
        mla_ui_control_t& __child = mla_array_list_get_unsafe((control).children, __i); \
        if (mla_ui_control_render_to_draw_commands(__childContext, __child, output)) {\
            return false; \
        } \
    } \
} \


#endif
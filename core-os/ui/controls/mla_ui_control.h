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
struct mla_ui_control_context_t;

typedef mla_ui_surface_draw_size_t (mla_ui_control_context_calcTextSize_t)(const mla_ui_control_context_t &context, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text);

struct mla_ui_control_context_t {
    mla_double_t offsetX;
    mla_double_t offsetY;
    mla_double_t width;
    mla_double_t height;
    mla_uint64_t timeSinceLastFrameMs;
    mla_user_data_t userData;
    mla_ui_surface_input_states_t input_states;
    mla_ui_control_context_calcTextSize_t *calcTextSize;
};

mla_ui_control_context_t mla_ui_control_context(mla_double_t width, mla_double_t height, const mla_ui_surface_input_states_t& input_states, mla_ui_control_context_calcTextSize_t *calcTextSize, mla_uint64_t timeSinceLastFrameMs, mla_user_data_t& userData);
mla_ui_control_context_t mla_ui_control_context(mla_double_t width, mla_double_t height, const mla_ui_surface_input_states_t& input_states, mla_ui_control_context_calcTextSize_t *calcTextSize, mla_uint64_t timeSinceLastFrameMs);
mla_ui_control_context_t mla_ui_control_create_context_for_children(const mla_ui_control_context_t &parentContext, const mla_ui_control_t &control);

struct mla_ui_control_value_t {
    mla_string_t name;
    mla_string_t stringValue;

    union {
        mla_int64_t int64Value;
        mla_uint64_t uint64Value;
        mla_double_t doubleValue;
        mla_bool_t boolValue;
        mla_platform_pointer_t pointerValue;
    };
};

mla_ui_control_value_t mla_ui_control_value_empty();

struct mla_ui_control_value_initializer_t {
    static mla_ui_control_value_t init() {
        return mla_ui_control_value_empty();
    }
};

struct mla_ui_control_input_area_t;
struct mla_ui_control_input_area_initializer_t;

struct mla_ui_control_initializer_t;

typedef mla_bool_t (*mla_ui_control_render_to_draw_commands_t)(const mla_ui_control_context_t &context, const mla_ui_control_t &element, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> &inputAreas);
typedef mla_bool_t (*mla_ui_control_process_click_event_t)(mla_ui_control_t &control, const mla_ui_surface_input_event_click_t &clickEvent, const mla_ui_control_input_area_t &inputArea, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_user_data_t& userData);
typedef mla_bool_t (*mla_ui_control_process_char_input_event_t)(mla_ui_control_t &control, const mla_ui_surface_input_event_char_input_t &charInputEvent, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_user_data_t& userData);

struct mla_ui_control_layout_t {
    mla_double_t x;
    mla_double_t y;
    mla_double_t width;
    mla_double_t height;
};

mla_ui_control_layout_t mla_ui_control_layout_empty();

struct mla_ui_control_t {
    mla_string_t id;
    mla_ui_control_layout_t layout;
    mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> children;
    mla_ui_control_render_to_draw_commands_t renderToDrawCommands;
    mla_ui_control_process_click_event_t processClickEvent;
    mla_ui_control_process_char_input_event_t processCharInputEvent;
    mla_array_list_t<mla_ui_control_value_t, mla_ui_control_value_initializer_t> values;
};

mla_ui_control_t mla_ui_control();
mla_ui_control_t mla_ui_control_empty();

struct mla_ui_control_initializer_t {
    static mla_ui_control_t init() {
        return mla_ui_control_empty();
    }
};




struct mla_ui_control_input_area_t {
    mla_string_t controlId;
    mla_ui_control_layout_t position; // x, y, width, height
    mla_string_t event_name;
    mla_user_data_t userData;
    mla_ui_surface_input_event_kind acceptedEvents; // Bitmask of accepted event kinds
};

mla_ui_control_input_area_t mla_ui_control_input_area_empty();
mla_ui_control_input_area_t mla_ui_control_input_area(const mla_string_t &controlId, mla_ui_control_layout_t position, const mla_string_t& event_name, mla_ui_surface_input_event_kind acceptedEvents);
mla_ui_control_input_area_t mla_ui_control_input_area(const mla_string_t &controlId, mla_ui_control_layout_t position, const mla_string_t& event_name, mla_ui_surface_input_event_kind acceptedEvents, mla_user_data_t& userData);

struct mla_ui_control_input_area_initializer_t {
    static mla_ui_control_input_area_t init() {
        return mla_ui_control_input_area_empty();
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
mla_platform_pointer_t mla_ui_control_get_value_as_pointer(const mla_ui_control_t &control, const mla_string_t &name, mla_platform_pointer_t defaultValue = nullptr);

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
mla_bool_t mla_ui_control_set_value_as_pointer(mla_ui_control_t &control, const mla_string_t &name, mla_platform_pointer_t value);

void mla_ui_control_reset_values(const mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, const mla_string_t &name);

mla_bool_t mla_ui_control_add_child(mla_ui_control_t &parent, const mla_ui_control_t &child);
mla_bool_t mla_ui_control_render_to_draw_commands(const mla_ui_control_context_t &context, const mla_ui_control_t &control, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> &inputAreas);
mla_bool_t mla_ui_controls_render_to_draw_commands(const mla_ui_control_context_t &context, const mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands, mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> &inputAreas);

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_t &control);
mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_layout_t &layout);

mla_bool_t mla_ui_control_has_focus(const mla_ui_control_t &control);

#define mla_ui_control_render_children_draw_commands(context, control, output, inputAreas) \
mla_size_t childrenCount = mla_array_list_size((control).children); \
if (childrenCount > 0) { \
    mla_ui_control_context_t __childContext = mla_ui_control_create_context_for_children(context, control); \
    for (mla_size_t __i = 0; __i < childrenCount; __i++) { \
        mla_ui_control_t& __child = mla_array_list_get_unsafe((control).children, __i); \
        if (!mla_ui_control_render_to_draw_commands(__childContext, __child, output, inputAreas)) {\
            return false; \
        } \
    } \
} \

mla_bool_t mla_ui_control_find_by_id(const mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, const mla_string_t &controlId, mla_ui_control_t* &outControl);
mla_bool_t mla_ui_control_find_focused_control(const mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, mla_ui_control_t* &outControl);

void mla_ui_control_process_input_events(mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t> &uiControls, const mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t> &inputEvents, const mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> &inputAreas, mla_user_data_t& userData);


#endif
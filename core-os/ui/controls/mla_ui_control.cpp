//
// Created by chris on 1/19/2026.
//

#include "mla_ui_control.h"
#include "../../system/mla_stream.h"
#include "../../system/mla_id.h"

mla_ui_control_value_t mla_ui_control_value_empty() {
    return {
        mla_string_empty(),
        mla_string_empty(),
        {0},
    };
}

mla_ui_control_t mla_ui_control() {

    return {
        mla_generate_runtime_id(),
{0,0,0,0},
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>(),
        nullptr,
        mla_array_list_empty<mla_ui_control_value_t, mla_ui_control_value_initializer_t>(),
    };
}

mla_ui_control_t mla_ui_control_empty() {
    return {
        mla_string_empty(),
        {0,0,0,0},
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>(),
        nullptr,
        mla_array_list_empty<mla_ui_control_value_t, mla_ui_control_value_initializer_t>(),
    };
}

mla_bool_t __mla_ui_control_find_value_by_name(const mla_ui_control_t &control, const mla_string_t &name, mla_ui_control_value_t &out) {

    for (mla_size_t i = 0; i < mla_array_list_size(control.values); i++) {
        mla_ui_control_value_t& value = mla_array_list_get_unsafe(control.values, i);
        if (mla_string_equals(value.name, name)) {
            out = value;
            return true;
        }
    }
    return false;
}

mla_uint8_t mla_ui_control_get_value_as_uint8(const mla_ui_control_t &control, const mla_string_t &name, mla_uint8_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_uint8_t>(value.uint64Value);
    }
    return defaultValue;

}

mla_int8_t mla_ui_control_get_value_as_int8(const mla_ui_control_t &control, const mla_string_t &name, mla_int8_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_int8_t>(value.int64Value);
    }
    return defaultValue;

}

mla_uint16_t mla_ui_control_get_value_as_uint16(const mla_ui_control_t &control, const mla_string_t &name, mla_uint16_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_uint16_t>(value.uint64Value);
    }
    return defaultValue;

}

mla_int16_t mla_ui_control_get_value_as_int16(const mla_ui_control_t &control, const mla_string_t &name, mla_int16_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_int16_t>(value.int64Value);
    }
    return defaultValue;

}

mla_uint32_t mla_ui_control_get_value_as_uint32(const mla_ui_control_t &control, const mla_string_t &name, mla_uint32_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_uint32_t>(value.uint64Value);
    }
    return defaultValue;

}

mla_int32_t mla_ui_control_get_value_as_int32(const mla_ui_control_t &control, const mla_string_t &name, mla_int32_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_int32_t>(value.int64Value);
    }
    return defaultValue;

}

mla_uint64_t mla_ui_control_get_value_as_uint64(const mla_ui_control_t &control, const mla_string_t &name, mla_uint64_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return value.uint64Value;
    }
    return defaultValue;

}

mla_int64_t mla_ui_control_get_value_as_int64(const mla_ui_control_t &control, const mla_string_t &name, mla_int64_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return value.int64Value;
    }
    return defaultValue;

}

mla_float_t mla_ui_control_get_value_as_float(const mla_ui_control_t &control, const mla_string_t &name, mla_float_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return static_cast<mla_float_t>(value.doubleValue);
    }
    return defaultValue;

}

mla_double_t mla_ui_control_get_value_as_double(const mla_ui_control_t &control, const mla_string_t &name, mla_double_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return value.doubleValue;
    }
    return defaultValue;

}

mla_string_t mla_ui_control_get_value_as_string(const mla_ui_control_t &control, const mla_string_t &name, const mla_string_t &defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return value.stringValue;
    }
    return defaultValue;

}

mla_bool_t mla_ui_control_get_value_as_bool(const mla_ui_control_t &control, const mla_string_t &name, mla_bool_t defaultValue) {

    mla_ui_control_value_t value = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, value)) {
        return value.boolValue;
    }
    return defaultValue;

}

mla_bool_t mla_ui_control_set_value_as_uint8(mla_ui_control_t &control, const mla_string_t &name, mla_uint8_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.uint64Value = static_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = static_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_int8(mla_ui_control_t &control, const mla_string_t &name, mla_int8_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.int64Value = static_cast<mla_int64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = static_cast<mla_int64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_uint16(mla_ui_control_t &control, const mla_string_t &name, mla_uint16_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.uint64Value = static_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = static_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_int16(mla_ui_control_t &control, const mla_string_t &name, mla_int16_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.int64Value = static_cast<mla_int64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = static_cast<mla_int64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_uint32(mla_ui_control_t &control, const mla_string_t &name, mla_uint32_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.uint64Value = static_cast<mla_uint64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = static_cast<mla_uint64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_int32(mla_ui_control_t &control, const mla_string_t &name, mla_int32_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.int64Value = static_cast<mla_int64_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = static_cast<mla_int64_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_uint64(mla_ui_control_t &control, const mla_string_t &name, mla_uint64_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.uint64Value = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.uint64Value = value;
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_int64(mla_ui_control_t &control, const mla_string_t &name, mla_int64_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.int64Value = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.int64Value = value;
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_float(mla_ui_control_t &control, const mla_string_t &name, mla_float_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.doubleValue = static_cast<mla_double_t>(value);
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.doubleValue = static_cast<mla_double_t>(value);
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_double(mla_ui_control_t &control, const mla_string_t &name, mla_double_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.doubleValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.doubleValue = value;
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_string(mla_ui_control_t &control, const mla_string_t &name, const mla_string_t &value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.stringValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.stringValue = value;
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_set_value_as_bool(mla_ui_control_t &control, const mla_string_t &name, mla_bool_t value) {

    mla_ui_control_value_t internalValue = mla_ui_control_value_empty();
    if (__mla_ui_control_find_value_by_name(control, name, internalValue)) {
        internalValue.boolValue = value;
        return true;
    }

    // Not found, create new
    mla_ui_control_value_t newValue = mla_ui_control_value_empty();
    newValue.name = name;
    newValue.boolValue = value;
    return mla_array_list_add(control.values, newValue);

}

mla_bool_t mla_ui_control_add_child(mla_ui_control_t &parent, const mla_ui_control_t &child) {
    return mla_array_list_add(parent.children, child);
}

mla_bool_t mla_ui_control_render_to_draw_commands(const mla_ui_control_context_t &context, const mla_ui_control_t &control, mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>& drawCommands) {

    if (control.renderToDrawCommands != nullptr) {
        return control.renderToDrawCommands(context, control, drawCommands);
    }
    return false;
}

mla_ui_control_context_t mla_ui_control_context(mla_size_t width, mla_size_t height, const mla_ui_surface_input_states_t& input_states, mla_ui_control_context_calcTextSize_t *calcTextSize) {
    return {
        0,
        0,
        width,
        height,
        0,
        input_states,
        calcTextSize,
    };
}

mla_ui_control_context_t mla_ui_control_create_context_for_children(const mla_ui_control_context_t &parentContext, const mla_ui_control_t &control) {

    mla_ui_control_context_t context = parentContext;

    if (control.layout.width != 0)
        context.width = control.layout.width;

    if (control.layout.height != 0)
        context.height = control.layout.height;

    return context;
}

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_layout_t &layout) {

    // If cursor position is negative, it's outside the surface
    if (context.input_states.cursorPosition.x < 0 || context.input_states.cursorPosition.y < 0) {
        return false;
    }

    mla_size_t mouseX = context.input_states.cursorPosition.x;
    mla_size_t mouseY = context.input_states.cursorPosition.y;

    mla_size_t controlX = layout.x + context.offsetX;
    mla_size_t controlY = layout.y + context.offsetY;
    mla_size_t controlW = layout.width;
    mla_size_t controlH = layout.height;

    if (controlW == 0) {
        controlW = context.width - layout.x;
    }

    if (controlH == 0) {
        controlH = context.height - layout.y;
    }

    return (mouseX >= controlX && mouseX <= (controlX + controlW) &&
            mouseY >= controlY && mouseY <= (controlY + controlH));

}

mla_bool_t mla_ui_control_is_hovered(const mla_ui_control_context_t& context, const mla_ui_control_t &control) {
    return mla_ui_control_is_hovered(context, control.layout);
}
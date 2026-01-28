//
// Created by chris on 1/26/2026.
//

#include "mla_ui_control_surface.h"
#include "../../system/mla_buffer.h"
#include "../../task/mla_task_manager.h"
#include "../../system/mla_id.h"

#define mla_ui_control_surface_drawing_lock_timeout_ms 100
#define mla_ui_control_surface_rendering_lock_timeout_ms 50

mla_ui_control_surface_drawing_t mla_ui_control_surface_drawing_empty() {
    return {
    mla_array_list_empty<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>(),
        mla_array_list_empty<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>()
    };
}

mla_ui_control_surface_rendering_t mla_ui_control_surface_rendering_empty() {
    return {
        mla_array_list_empty<mla_ui_control_t, mla_ui_control_initializer_t>(),
        mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>(),
        nullptr
    };
}

mla_ui_control_surface_t mla_ui_control_surface_empty() {
    return {
        mla_string_empty(),
        mla_buffer_reference_noOwner(),
        mla_ui_surface_invalid(),
        mla_mutex_invalid(),
        mla_ui_control_surface_rendering_empty(),
        mla_ui_control_surface_drawing_empty(),
        0
    };
}

mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface) {
    return mla_ui_control_surface_create(surface, 0);
}

mla_ui_control_surface_t mla_ui_control_surface_create(const mla_ui_surface_t &surface, mla_callback_userdata userData) {
    return {
        mla_string_empty(),
        mla_buffer_reference_noOwner(),
        surface,
        mla_mutex_create("mla_ui_control_surface_lock"),
        mla_ui_control_surface_rendering_empty(),
        mla_ui_control_surface_drawing_empty(),
        userData
    };
}

mla_string_t __mla_ui_control_surface_rendering_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_rendering_", taskId);
}

mla_string_t __mla_ui_control_surface_drawing_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_drawing_", taskId);
}

mla_string_t __mla_ui_control_surface_rendering_drawing_task_name(const mla_string_t &taskId) {
    return mla_string_concat("surface_rendering_drawing", taskId);
}

mla_buffer_cleanup_mode __mla_ui_control_surface_cleanup(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;

    mla_ui_control_surface_t* connector = static_cast<mla_ui_control_surface_t*>(data);

    if (connector) {
        // Abort rendering task
        if (!mla_string_is_empty(connector->taskId)) {
            mla_task_manager_abort_task(__mla_ui_control_surface_rendering_task_name(connector->taskId));
            mla_task_manager_abort_task(__mla_ui_control_surface_drawing_task_name(connector->taskId));
        }
    }

    // No cleanup needed for this resource. We just need to stop the tasks.
    return CLEAN_UP_SKIP;
}

mla_buffer_cleanup_mode __mla_ui_control_surface_cleanup_single_thread(mla_pointer_t data, mla_callback_userdata userData) {
    (void) userData;

    mla_ui_control_surface_t* connector = static_cast<mla_ui_control_surface_t*>(data);

    if (connector) {
        // Abort rendering task
        if (!mla_string_is_empty(connector->taskId)) {
            mla_task_manager_abort_task(__mla_ui_control_surface_rendering_drawing_task_name(connector->taskId));
        }
    }

    // No cleanup needed for this resource. We just need to stop the tasks.
    return CLEAN_UP_SKIP;
}

mla_ui_surface_draw_size_t __mla_ui_control_surface_calc_text_size(const mla_ui_control_context_t &context, const mla_ui_surface_font_type_t &font_type, const mla_string_t &text) {

    mla_ui_control_surface_t* connector = reinterpret_cast<mla_ui_control_surface_t*>(context.userData);

    if (connector->surface.calc_text_size == nullptr) {
        return {0, 0};
    }

    return connector->surface.calc_text_size(connector->surface, font_type, text);
}

mla_task_process_result_state __mla_ui_control_surface_render_task(mla_callback_userdata userData) {

    mla_ui_control_surface_t* connector = reinterpret_cast<mla_ui_control_surface_t*>(userData);

    mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t> unprocessedInputEvents = mla_array_list_empty<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>();

    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_rendering_lock_timeout_ms)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    unprocessedInputEvents = connector->drawing.unprocessedInputEvents;
    connector->drawing.unprocessedInputEvents = mla_array_list_empty<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>();

    // Unlock while rendering
    mla_mutex_unlock(connector->lock);

    // Get surface size and input states
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(connector->surface);
    mla_ui_surface_input_states_t input_states = mla_ui_surface_get_input_states(connector->surface);

    // Process input events
    mla_ui_control_process_input_events(connector->rendering.root, unprocessedInputEvents, connector->rendering.inputAreas, connector->userData);

    if (connector->rendering.renderingTask) {

        // Call custom rendering task if provided
        if (!connector->rendering.renderingTask(connector->rendering.root, surfaceSize, input_states)) {
            mla_error("Custom rendering task failed");
            return TASK_PROCESS_RESULT_CONTINUE;
        }
    }

    // Update the draw commands
    mla_ui_control_context_t context = mla_ui_control_context(surfaceSize.width, surfaceSize.height, input_states, __mla_ui_control_surface_calc_text_size, userData);
    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> drawCommands = mla_array_list<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t>();
    mla_array_list_t<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t> inputAreas = mla_array_list<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();


    if (!mla_ui_controls_render_to_draw_commands(context, connector->rendering.root, drawCommands, inputAreas)) {
        mla_error("Failed to render UI controls to draw commands");
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Lock again to update shared data
    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_rendering_lock_timeout_ms)) {
        mla_warning("Failed to lock mutex to update draw commands");
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Update drawing data
    connector->drawing.drawCommands = drawCommands;
    connector->rendering.inputAreas = inputAreas;

    mla_mutex_unlock(connector->lock);

    return TASK_PROCESS_RESULT_CONTINUE;
}

mla_task_process_result_state __mla_ui_control_surface_drawing_task(mla_callback_userdata userData) {

    mla_ui_control_surface_t* connector = reinterpret_cast<mla_ui_control_surface_t*>(userData);

    if (!mla_mutex_trylock(connector->lock, mla_ui_control_surface_drawing_lock_timeout_ms)) {
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    mla_array_list_t<mla_ui_surface_draw_command_t, mla_ui_surface_draw_command_initializer_t> drawCommands = connector->drawing.drawCommands;
    mla_mutex_unlock(connector->lock);

    mla_array_list_t<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t> newUnprocessedInputEvents = mla_array_list_empty<mla_ui_surface_input_event_t, mla_ui_surface_input_event_initializer_t>();

    // Render to surface
    mla_ui_surface_render_draw_commands(
        connector->surface,
        drawCommands,
        newUnprocessedInputEvents
    );

    if (!mla_mutex_lock(connector->lock)) {
        mla_warning(mla_string_const("Unable to lock mutex to update input events"));
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Append new unprocessed input events
    mla_array_list_add_all(connector->drawing.unprocessedInputEvents, newUnprocessedInputEvents);

    mla_mutex_unlock(connector->lock);

    return TASK_PROCESS_RESULT_CONTINUE;
}

mla_task_process_result_state __mla_ui_control_surface_render_and_draw_task(mla_callback_userdata userData) {

    mla_ui_control_surface_t* connector = reinterpret_cast<mla_ui_control_surface_t*>(userData);

    // No locking here as it's single threaded mode

    // Get surface size and input states
    mla_ui_surface_size_t surfaceSize = mla_ui_surface_get_size(connector->surface);
    mla_ui_surface_input_states_t input_states = mla_ui_surface_get_input_states(connector->surface);

    // Process input events
    mla_ui_control_process_input_events(connector->rendering.root, connector->drawing.unprocessedInputEvents, connector->rendering.inputAreas, connector->userData);
    mla_array_list_clear(connector->drawing.unprocessedInputEvents);

    if (connector->rendering.renderingTask) {

        // Call custom rendering task if provided
        if (!connector->rendering.renderingTask(connector->rendering.root, surfaceSize, input_states)) {
            mla_error("Custom rendering task failed");
            return TASK_PROCESS_RESULT_CONTINUE;
        }
    }

    // Update the draw commands
    mla_ui_control_context_t context = mla_ui_control_context(surfaceSize.width, surfaceSize.height, input_states, __mla_ui_control_surface_calc_text_size, userData);

    mla_array_list_clear(connector->drawing.drawCommands);
    mla_array_list_clear(connector->rendering.inputAreas);

    if (!mla_ui_controls_render_to_draw_commands(context, connector->rendering.root, connector->drawing.drawCommands, connector->rendering.inputAreas)) {
        mla_error("Failed to render UI controls to draw commands");
        return TASK_PROCESS_RESULT_CONTINUE;
    }

    // Render to surface
    mla_ui_surface_render_draw_commands(
        connector->surface,
        connector->drawing.drawCommands,
        connector->drawing.unprocessedInputEvents
    );

    return TASK_PROCESS_RESULT_CONTINUE;

}

mla_bool_t mla_ui_control_surface_start(mla_ui_control_surface_t &connector, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t>& root, const mla_ui_control_surface_rendering_task_t &renderingTask) {

    if (connector.rendering.renderingTask) {
        mla_warning("Connector is already started");
        return false;
    }

    // Init rendering
    connector.rendering.root = root;
    connector.rendering.renderingTask = renderingTask;
    connector.rendering.inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    mla_string_t id = mla_generate_runtime_id();

    // Create rendering task
    mla_task_t render_task = mla_task_repeating(
            __mla_ui_control_surface_rendering_task_name(id),
            __mla_ui_control_surface_render_task,
            reinterpret_cast<mla_callback_userdata>(&connector)
        );

    // Create the Drawing Task
    mla_task_t drawing_task = mla_task_repeating(
            __mla_ui_control_surface_drawing_task_name(id),
            __mla_ui_control_surface_drawing_task,
            reinterpret_cast<mla_callback_userdata>(&connector)
        );

    // Create rendering thread
    if (!mla_task_manager_register_task(render_task)) {
        mla_error("Failed to register UI control surface connector rendering task!");
        return false;
    }

    // Create drawing thread
    if (!mla_task_manager_register_task(drawing_task)) {
        mla_error("Failed to register UI control surface connector drawing task!");

        // Abort previously created rendering task
        mla_task_manager_abort_task(render_task.name);
        return false;
    }

    // Store task reference
    connector.task_cleanup = mla_buffer_reference(&connector, true, __mla_ui_control_surface_cleanup);
    connector.taskId = id;

    return true;
}

mla_bool_t mla_ui_control_surface_start_single_threaded_mode(mla_ui_control_surface_t &connector, mla_array_list_t<mla_ui_control_t, mla_ui_control_initializer_t>& root, const mla_ui_control_surface_rendering_task_t &renderingTask) {

    if (connector.rendering.renderingTask) {
        mla_warning("Connector is already started");
        return false;
    }

    // Init rendering
    connector.rendering.root = root;
    connector.rendering.renderingTask = renderingTask;
    connector.rendering.inputAreas = mla_array_list_empty<mla_ui_control_input_area_t, mla_ui_control_input_area_initializer_t>();

    mla_string_t id = mla_generate_runtime_id();

    // Create rendering task
    mla_task_t render_draw_task = mla_task_repeating(
            __mla_ui_control_surface_rendering_drawing_task_name(id),
            __mla_ui_control_surface_render_and_draw_task,
            reinterpret_cast<mla_callback_userdata>(&connector)
        );

    if (!mla_task_manager_register_task(render_draw_task)) {
        mla_error("Failed to register UI control surface connector rendering/drawing task!");
        return false;
    }

    // Store task reference
    connector.task_cleanup = mla_buffer_reference(&connector, true, __mla_ui_control_surface_cleanup);
    connector.taskId = id;

    return true;
}

mla_bool_t mla_ui_control_surface_stop(mla_ui_control_surface_t &connector) {

    // Clear rendering task
    connector.task_cleanup = mla_buffer_reference_noOwner();
    return true;
}

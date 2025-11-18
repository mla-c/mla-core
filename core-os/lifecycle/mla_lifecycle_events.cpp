
#include "mla_lifecycle_events.h"

#include "../log/mla_logging.h"
#include "../system/mla_array_list.h"

struct mla_lifecycle_event_t {
    mla_int32_t priority;
    mla_lifecycle_event_callback_t callback;
};

struct mla_lifecycle_event_manager_t {
    mla_array_list_t<mla_lifecycle_event_t> boot_events;
    mla_bool_t boot_events_fired;
};

static mla_lifecycle_event_manager_t g_lifecycle_event_manager = {
    mla_array_list_empty<mla_lifecycle_event_t>(),
    false
};

void mla_lifecycle_boot_event_register(mla_int32_t priority, mla_lifecycle_event_callback_t callback) {

    if (g_lifecycle_event_manager.boot_events_fired) {
        mla_warning("Attempted to register boot event after boot events have been fired. Ignoring.");
        return;
    }

    mla_lifecycle_event_t event = {
        priority,
        callback
    };
    mla_array_list_add(g_lifecycle_event_manager.boot_events, event);
}


mla_int32_t __mla_lifecycle_event_sort(const mla_lifecycle_event_t& a, const mla_lifecycle_event_t& b) {
    // Lower number is higher priority
    if (a.priority < b.priority) {
        return -1;
    } else if (a.priority > b.priority) {
        return 1;
    } else {
        return 0;
    }
}

void mla_lifecycle_fire_boot_events() {

    if (g_lifecycle_event_manager.boot_events_fired) {
        mla_warning("Boot events have already been fired. Ignoring subsequent call.");
        return;
    }

    g_lifecycle_event_manager.boot_events_fired = true;

    // Sort events by priority
    mla_array_list_sort(g_lifecycle_event_manager.boot_events, __mla_lifecycle_event_sort);

    // Fire events in order
    for (mla_size_t i = 0; i < mla_array_list_size(g_lifecycle_event_manager.boot_events); ++i) {

        mla_lifecycle_event_t& event = mla_array_list_get_unsafe(g_lifecycle_event_manager.boot_events, i);
        if (event.callback != nullptr) {
            event.callback();
        }
    }

    // Clear the events after firing
    mla_array_list_clear(g_lifecycle_event_manager.boot_events);
}
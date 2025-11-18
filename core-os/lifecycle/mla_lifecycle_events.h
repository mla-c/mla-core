//
// Created by christian on 11/18/25.
//

#ifndef COREOS_MLA_LIFECYCLE_EVENTS_H
#define COREOS_MLA_LIFECYCLE_EVENTS_H
#include "../mla_data_types.h"

typedef void (*mla_lifecycle_event_callback_t)(void);

#define mla_lifecycle_boot_event_priority_low_level_setup (-100)
#define mla_lifecycle_boot_event_priority_file_system_setup (0)
#define mla_lifecycle_boot_event_priority_network_setup (100)
#define mla_lifecycle_boot_event_priority_application_setup (200)

void mla_lifecycle_boot_event_register(mla_int32_t priority, mla_lifecycle_event_callback_t callback);
void mla_lifecycle_fire_boot_events();

#define mla_lifecycle_boot_event_static_register(priority, callback_name) \
    struct MlaLifecycleBootEventRegister_##callback_name { \
        MlaLifecycleBootEventRegister_##callback_name() { \
            mla_lifecycle_boot_event_register(priority, callback_name); \
        } \
    }; \
    static MlaLifecycleBootEventRegister_##callback_name mla_lifecycle_boot_event_register_instance_##callback_name; \

#endif
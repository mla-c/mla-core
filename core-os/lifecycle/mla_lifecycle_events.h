//
// Created by christian on 11/18/25.
//

#ifndef MLA_C_MLA_LIFECYCLE_EVENTS_H
#define MLA_C_MLA_LIFECYCLE_EVENTS_H
#include "../mla_data_types.h"

typedef void (*mla_lifecycle_event_callback_t)(void);

#define mla_lifecycle_boot_event_priority_low_level_preSetup (-1010)
#define mla_lifecycle_boot_event_priority_low_level_setup (-1000)
#define mla_lifecycle_boot_event_priority_low_level_postSetup (-990)

#define mla_lifecycle_boot_event_priority_file_system_preSetup (-10)
#define mla_lifecycle_boot_event_priority_file_system_setup (0)
#define mla_lifecycle_boot_event_priority_file_system_postSetup (10)

#define mla_lifecycle_boot_event_priority_rpc_preSetup (490)
#define mla_lifecycle_boot_event_priority_rpc_setup (500)
#define mla_lifecycle_boot_event_priority_rpc_postSetup (510)

#define mla_lifecycle_boot_event_priority_network_preSetup (990)
#define mla_lifecycle_boot_event_priority_network_setup (1000)
#define mla_lifecycle_boot_event_priority_network_postSetup (1100)

#define mla_lifecycle_boot_event_priority_application_preSetup (1900)
#define mla_lifecycle_boot_event_priority_application_setup (2000)
#define mla_lifecycle_boot_event_priority_application_postSetup (2100)

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
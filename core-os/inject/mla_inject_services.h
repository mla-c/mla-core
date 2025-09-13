//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_INJECT_SERVICES_H
#define COREOS_MLA_INJECT_SERVICES_H

#include "../system/mla_string.h"

struct mla_inject_bootstrap_service_t {

    static mla_string_t get_service_name() {
        static mla_string_t serviceName = mla_string("BootStrapService");
        return serviceName;
    }

    mla_string_t real_service_name;
    void (*setup)(mla_inject_bootstrap_service_t* instance);
    void (*teardown)(mla_inject_bootstrap_service_t* instance);
    mla_callback_userdata userdata;
};

#endif

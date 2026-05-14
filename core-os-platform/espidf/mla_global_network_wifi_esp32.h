//
// Created by chris on 3/8/2026.
//

#ifndef MLA_GLOBAL_NETWORK_WIFI_ESP32_H
#define MLA_GLOBAL_NETWORK_WIFI_ESP32_H

#include "../../core-os/mla_data_types.h"
#include "../../core-os/config/mla_config.h"
#include "../../core-os/reflection/mla_reflection.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

struct mla_global_network_wifi_esp32_t {
    mla_string_t ssid;
    mla_string_t password;


    static mla_bool_t serialize(mla_serializer_t &serializer, const mla_platform_pointer_t obj) {
        const mla_global_network_wifi_esp32_t *self = static_cast<const mla_global_network_wifi_esp32_t *>(obj);
        mla_serializer_write_string(serializer, mla_string_const("ssid"), self->ssid);
        mla_serializer_write_string(serializer, mla_string_const("password"), self->password);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(
        mla_deserializer_t &deserializer,
        mla_platform_pointer_t obj,
        const mla_string_t &property_name) {
        mla_global_network_wifi_esp32_t *self = static_cast<mla_global_network_wifi_esp32_t *>(obj);

        if (mla_string_equals_const(property_name, "ssid")) {
            mla_deserializer_read_string(deserializer, self->ssid);
        } else if (mla_string_equals_const(property_name, "password")) {
            mla_deserializer_read_string(deserializer, self->password);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        auto data = mla_reflection_struct_metadata(mla_global_network_wifi_esp32_t);
        mla_reflection_struct_field_string(data, mla_global_network_wifi_esp32_t, ssid);
        mla_reflection_struct_field_string(data, mla_global_network_wifi_esp32_t, password);
        mla_reflection_struct_metadata_freeze(data);
        return data;
    }

    static mla_global_network_wifi_esp32_t init() {
        return {
            mla_string_empty(),
            mla_string_empty()
        };
    }
};

mla_reflection_auto_register_struct(mla_global_network_wifi_esp32_t)

mla_config_definition_t mla_global_network_wifi_esp32_config_definition() {
    return mla_config_definition<mla_global_network_wifi_esp32_t>(
        mla_string_const("wifi"),
        mla_serialize_definition<mla_global_network_wifi_esp32_t>()
    );
}

void __mla_global_network_wifi_esp32_init() {

    // Read the wifi config
    mla_global_network_wifi_esp32_t wifiConfig = mla_global_network_wifi_esp32_t::init();

    if (mla_config_manager_read(mla_global_network_wifi_esp32_config_definition(), wifiConfig)) {

        // Connect to WiFi
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        wifi_config_t esp_wifi_config = {};

        // Copy SSID
        size_t ssid_len = mla_string_length(wifiConfig.ssid) < 32 ? mla_string_length(wifiConfig.ssid) : 31;
        mla_memcpy(esp_wifi_config.sta.ssid, mla_string_data(wifiConfig.ssid), ssid_len);
        esp_wifi_config.sta.ssid[ssid_len] = '\0';

        // Copy Password
        size_t pass_len = mla_string_length(wifiConfig.password) < 64 ? mla_string_length(wifiConfig.password) : 63;
        mla_memcpy(esp_wifi_config.sta.password, mla_string_data(wifiConfig.password), pass_len);
        esp_wifi_config.sta.password[pass_len] = '\0';

        // Improve security threshold
        esp_wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        esp_wifi_config.sta.pmf_cfg.capable = true;
        esp_wifi_config.sta.pmf_cfg.required = false;

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &esp_wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        esp_wifi_connect();

    } else {

        // Create a wifi station with default config to allow configuration via captive portal
        // Usually involves setting up SoftAP mode
        esp_netif_create_default_wifi_ap();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        wifi_config_t wifi_ap_config = {};
        const char* ap_ssid = "CORE_OS_INIT";
        const char* ap_pass = ""; // Open network or set default password

        mla_memcpy(wifi_ap_config.ap.ssid, ap_ssid, strlen(ap_ssid));
        wifi_ap_config.ap.ssid_len = strlen(ap_ssid);
        wifi_ap_config.ap.channel = 1;
        wifi_ap_config.ap.max_connection = 4;
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;

        if (mla_strlen(ap_pass) > 0) {
            mla_memcpy(wifi_ap_config.ap.password, ap_pass, strlen(ap_pass));
            wifi_ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        }

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
        ESP_ERROR_CHECK(esp_wifi_start());

    }


}

mla_lifecycle_boot_event_static_register(mla_lifecycle_boot_event_priority_network_postSetup, __mla_global_network_wifi_esp32_init)



#endif

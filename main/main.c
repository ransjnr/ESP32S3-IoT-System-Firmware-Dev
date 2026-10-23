#include <stdio.h>
#include "test_component.h"
#include "freertos/FreeRTOS.h"
#include "app_state.h"
#include "esp_log.h"
#include "wifi-imp.h"
#include "nvs_flash.h"
#include "http_imp.h"
#include "telemetry.h"

static const char* TAG = "main";

void app_main(void)
{
    app_state_init();
    app_set_state(APP_STATE_INIT);

    while (1){
        /* code */
        switch(app_get_state())
        {
        case APP_STATE_INIT:
            /*code */
            ESP_LOGI(TAG, "State: INIT");
            esp_err_t ret = nvs_flash_init();
            if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
            }
            ESP_ERROR_CHECK(ret);
            ESP_LOGI(TAG, "NVS Flash initialized successfully");
            app_set_state(APP_STATE_WIFI_CONNECT);
            break;
        case APP_STATE_WIFI_CONNECT:
            /* code */
            wifi_init(); // initialize the wifi module and allow the firmware to check for the credentials in the NVS
            break;
        case APP_STATE_WIFI_FROM_NVS:
            /* code */
            ESP_LOGI(TAG, "State: WIFI_FROM_NVS");
            wifi_init_station(); // initialize the wifi module in station mode
            app_set_state(APP_STATE_RUNNING);
            break;
        case APP_STATE_WIFI_PROVISIONING:
            /* code */
            ESP_LOGI(TAG, "State: WIFI_PROVISIONING");
            start_ap_mode(); // start AP mode so that user can enter the credentials through the web interface
            // start the locally deployed http server to allow the user to enter the credentials through the web interface
            http_wifi_credentials_server_init();
            app_set_state(APP_STATE_RUNNING);
            break;
        case APP_STATE_TELEMETRY_INIT:
            /* code */
            ESP_LOGI(TAG, "State: TELEMETRY_INIT");

            if (telemetry_init() != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to initialize telemetry system");
                vTaskDelay(pdMS_TO_TICKS(2000)); // wait for 1 second before restarting
                app_set_state(APP_STATE_RESTART);
            }
            // telemetry_init(); // Initialize the telemetry system
            else
            {
                app_set_state(APP_STATE_RUNNING);
            }
            break;
        case APP_STATE_RESTART:
            /* code */
            ESP_LOGE(TAG, "State: RESTART");
            esp_restart(); // restart the device
            break;
        case APP_STATE_RUNNING:
            /* code */
            ESP_LOGI(TAG, "State: Running");
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        default:
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

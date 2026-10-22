#include <stdio.h>
#include "test_component.h"
#include "freertos/FreeRTOS.h"
#include "app_state.h"
#include "esp_log.h"
#include "wifi-imp.h"
#include "nvs_flash.h"

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
            wifi_init_station();
            app_set_state(APP_STATE_RUNNING);
            break;
        case APP_STATE_TELEMETRY_INIT:
            /* code */
            ESP_LOGI(TAG, "State: TELEMETRY_INIT");
            app_set_state(APP_STATE_RUNNING);
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

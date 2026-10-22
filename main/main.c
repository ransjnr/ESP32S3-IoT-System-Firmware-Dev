#include <stdio.h>
#include "test_component.h"
#include "freertos/FreeRTOS.h"
#include "app_state.h"
#include "esp_log.h"

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
            app_set_state(APP_STATE_RUNNING);
            break;
        case APP_STATE_WIFI_CONNECT:
            /* code */
            break;
        case APP_STATE_TELEMETRY_INIT:
            /* code */
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

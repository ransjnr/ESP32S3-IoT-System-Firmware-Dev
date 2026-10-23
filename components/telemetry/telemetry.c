#include <stdio.h>
#include "telemetry.h"
#include "esp_log.h"

#define QUEUE_LENGTH 10
static const char* TAG = "Telemetry";
static QueueHandle_t telemetry_queue = NULL;

esp_err_t telemetry_init(void)
{
    // Initialize telemetry queue or any other necessary resources
    telemetry_queue = xQueueCreate(QUEUE_LENGTH, sizeof(sensor_data_t));
    if (telemetry_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create telemetry queue");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Telemetry Queue Ready!");
    return ESP_OK;
}

BaseType_t telemetry_enque(int8_t temperature, uint8_t humidity)
{
    if(!telemetry_queue)
    {
        ESP_LOGE(TAG, "Telemetry queue not initialized");
        return pdFAIL;
    }
    sensor_data_t data = {.temperature = temperature, .humidity = humidity}; // Create a sensor_data_t struct with the provided temperature and humidity
    return xQueueSend(telemetry_queue, &data, pdMS_TO_TICKS(100)); // Enqueue the sensor data with a timeout of 100 ms
}

BaseType_t telemetry_deuque(sensor_data_t* out, TickType_t tick)
{
    // Dequeue the sensor data
    if (!telemetry_queue || !out)
    {
        ESP_LOGE(TAG, "Telemetry queue not initialized or output pointer is NULL");
        return pdFAIL;
    }
    return xQueueReceive(telemetry_queue, out, tick); // Dequeue the sensor
}
